#include "user.h"

#include "mario_walking.h"
#include "mario_jumping.h"
#include "mario_standing.h"

#include "saiyan_walking.h"
#include "saiyan_jumping.h"
#include "saiyan_standing.h"
#include "question_block.h"
#include "mushroom.h"
#include "overworld.h"

#define DUCK_HRZ_1 (0x03)
#define DUCK_HRZ_2 (0x04)
#define DUCK_HRZ_3 (0x05)
#define BG_OAM (0x7A)

#define SCREEN_X_MIN (0)
#define SCREEN_Y_MIN (0)
#define SCREEN_X_MAX (640 - 64 - 1)
#define SCREEN_Y_MAX (480 - 64 - 1)



#define GROUND (SCREEN_Y_MAX - 75)
#define JUMP_HEIGHT 50
#define STATIC_POS 80

#define NONE 0
#define DOWN 1
#define UP 2
#define LEFT 4
#define RIGHT 8

#define OAM_MARIO_STAND (uint8_t)0
#define OAM_MARIO_WALK (uint8_t)1
#define OAM_MARIO_JUMP (uint8_t)2

#define STALL (uint8_t)4



bool game_over = false;

typedef struct player_t {
    uint8_t side;
    bool left;
    bool right;
    bool up;
    bool jump;
    uint16_t jump_height;
    uint8_t contact;
    uint16_t screen_start;
    uint16_t x;
    uint16_t y;
    uint8_t dx;
    uint8_t dy;
    uint8_t stall;
    uint8_t oam_curr_id;
    uint8_t oam_base_id;
} player_t;

player_t p1 = {
    .side = 0, //only one input player
    .left = false,
    .right = false,
    .up = false,
    .jump = true,
    .jump_height = 0,
    .contact = NONE,
    .screen_start = 0,
    .x = 100,
    .y = GROUND,
    .dx = 2,
    .dy = 2,
    .stall = 0
};

void get_input(void* data) {
    player_t *player = (player_t*) data;

    sos_input_state_t state;
    sos_input_id_t id = sos_get_input_id(player->side);

    sos_fill_input_state(id, &state);
    player->left = state.left;
    //player->right = state.right;
    player->up = state.up;
    if (state.left) {
        sos_uart_printf("left pressed\n");
    }
    if (state.right) {
        sos_uart_printf("right cpressed\n");
    }
    if (player->up) {
        sos_uart_printf("UP!\r\n");
    }
}


void swap_character(player_t* p, uint8_t new_oam) {
    //disable old oam
    sos_oam_update(p->oam_curr_id,
        SOS_OAM_ENABLE = false;
    );
    p->oam_curr_id = p->oam_base_id + new_oam;
    sos_oam_update(p->oam_curr_id,
        SOS_OAM_OFFSET_Y = p->y;
    );
    sos_oam_update(p->oam_curr_id,
        SOS_OAM_OFFSET_X = p->x;
    );
    sos_oam_update(p->oam_curr_id,
        SOS_OAM_ENABLE = true;
    );
    if (p->right) {
        sos_oam_update(p->oam_curr_id,
        SOS_OAM_FLIP_X = true;
        );
    }
}

void move_right(player_t *p) {
    if ( !(p->contact && RIGHT) ) {
        /*if (p->x < STATIC_POS - p->dx) {
            p->x += p->dx;
        } else {
            p->x = STATIC_POS;
            p->screen_pos += p->dx;
            // !!! mario stays put. objects move closer !!!
        }*/
        if (p->x + p->dx < SCREEN_X_MAX) {
            p->x += p->dx;
        }
        else
            p->x = SCREEN_X_MAX;
    }
    if ( p->stall > STALL) {
        if (p->oam_base_id - p->oam_curr_id == OAM_MARIO_STAND) {
            swap_character(p, OAM_MARIO_JUMP);
            p->stall = 0;
        } else { 
            swap_character(p, OAM_MARIO_STAND);
            p->stall = 0;
        }
    } else {
        p->stall++;
    }
}

void move_left(player_t *p) {
    if ( !(p->contact && LEFT) ) {
        /*if (p->x > SCREEN_X_MIN + p->dx)
            p->x -= p->dx;
        else
            p->x = SCREEN_X_MIN;
        else {
            //do nothing
        }*/
        if (p->x - p->dx < SCREEN_X_MIN) {
            //p->x = SCREEN_X_MIN;
        }
        else
            p->x -= p->dx;
    }
    // swap standing/walking
}

void player_update(void* data) {
    player_t *p = (player_t*) data;
    if (p->left && p->right) {
        // do nothing
    } else if (p->right) {
        move_right(p);
    } else if (p->left) {
        move_left(p);
    } else if (p->up) {
        if (!p->jump) {
            p->jump = true;
            p->jump_height = 0;
            p->y += p->dy;
            
        }
    }
    
    if (p->jump) {
        swap_character(p, OAM_MARIO_JUMP);
        if (p->jump_height < JUMP_HEIGHT) {
            if ( p->contact & UP ) { 
                p->jump_height = JUMP_HEIGHT;
            } else {
                p->y -= p->dy;
                p->jump_height += p->dy;
            }
        } 
        else {
            if ( p->y >= GROUND || p->contact & DOWN) {
                    p->jump = false;
                p->jump_height = 0;
                swap_character(p, OAM_MARIO_STAND);
            } else {
                p->y += p->dy;
            }
        }
    }

    
    // Update OAM
    sos_oam_update(p->oam_curr_id,
        SOS_OAM_OFFSET_X = (uint16_t) p->x;
    );
    sos_oam_update(p->oam_curr_id,
        SOS_OAM_OFFSET_Y = (uint16_t) p->y;
    );
}


#define NUM_BOXES (4)
typedef struct mushroom_t {
    uint8_t id;
    bool active;
} mushroom_t;

typedef struct box_t {
    uint8_t id;
    bool hit;
    mushroom_t mushroom;
    short x;
    short y;
    short width;
    short height;
} box_t;
box_t boxes[NUM_BOXES];

uint32_t rand() {
    static uint32_t m_w = 91847;    /* must not be zero, nor 0x464fffff */
    static uint32_t m_z = 12987412;    /* must not be zero, nor 0x9068ffff */
    m_z = 36969 * (m_z & 65535) + (m_z >> 16);
    m_w = 18000 * (m_w & 65535) + (m_w >> 16);

    return (m_z << 16) + m_w;  /* 32-bit result */
}

// Given a <parts_per_32b_int> chance, maybe trigger
bool rand_trigger(uint32_t parts_per_32b_int) {
    uint32_t r = rand();
    return r <= parts_per_32b_int;
}


#define INCLUSIVE_BETWEEN(diff, low, high) ( (diff >= low) ? (diff <= high) : false)
void detect_mario_contact(void* data) {
    player_t *p = (player_t*) data;
    p->contact = NONE;
    for (box_t *b = &boxes[0]; b <= &boxes[NUM_BOXES]; b++) {
        uint16_t h = 0;
        uint16_t w = 0;
        //b->y -= 1;
        for (h = 0; h <= b->height; h++) { 
            if (INCLUSIVE_BETWEEN(b->x+22, p->x+40, p->x+41) && //mario is about 22 wide, in the middle of sprite
                INCLUSIVE_BETWEEN(b->y+32+h, p->y+32, p->y+65)) //maybe mario is 31 tall? probably needs to be lowered
            {
                p->contact |= RIGHT;
            }
        }
        for (h = 0; h < b->height; h++) { 
            if (INCLUSIVE_BETWEEN(b->x+b->width, p->x+20, p->x+22) && //mario is about 22 wide, in the middle of sprite
                INCLUSIVE_BETWEEN(b->y+h, p->y, p->y+31)) //maybe mario is 31 tall? probably needs to be lowered
            {
                p->contact |= LEFT;
            }
        }
        for (w = 0; w < b->width; w++) { 
             if (INCLUSIVE_BETWEEN(b->x+w+16, p->x+22, p->x+40) &&
                INCLUSIVE_BETWEEN(b->y+64, p->y+40, p->y+40)) 
            {
                p->contact |= UP;
                b->hit = true;
                sos_oam_update(b->id, 
                    SOS_OAM_ENABLE = false;
                );
                sos_oam_update(b->mushroom.id,
                    SOS_OAM_ENABLE = true;
                );
                
                //active_mushroom(b);
                sos_uart_printf("contact up\n");
            }
            if (INCLUSIVE_BETWEEN(b->x+w, p->x+22, p->x+44) && //mario is about 22 wide, in the middle of sprite
                INCLUSIVE_BETWEEN(b->y, p->y+64, p->y+66)) //maybe mario is 31 tall? probably needs to be lowered
            {
                p->contact |= DOWN;
            }       
        }       
        //sos_oam_update(b->id,
        //    SOS_OAM_OFFSET_Y = (uint16_t) b->y;
        //);
    }
}


#define KEEP_SIGN(num, sign) (num * ((sign < 0) ? -1 : 1))

#define NUM_CBS 8
sos_cb_id_t cb_ids[NUM_CBS];

// Register interupts, init graphics etc...
void sos_user_game_init() {
  // clear all oams
  for (uint8_t offset = 0x00; offset <= 0xF0; offset += 4) {
    (*(volatile uint32_t *)((0xA0000000 + offset)) = (0x00000000));
  }

    // load colors
    sos_cram_load_palette(0x00, overworld_palette);
    sos_cram_load_palette(0x02, mushroom_palette);
    sos_cram_load_palette(0x03, question_block_palette);
    sos_cram_load_palette(0x04, mario_standing_palette);
    sos_cram_load_palette(0x05, mario_walking_palette);
    sos_cram_load_palette(0x06, mario_jumping_palette);
    // these could all be using the same palette


    // load duck vram/oam & init duckstruct
    uint8_t curr_oam = 0x00;
    for (uint8_t i = 0; i < NUM_BOXES; i+=2) {
        boxes[i].id = i;
        boxes[i].hit = false;
        boxes[i].x = 100+65*i;
        boxes[i].y = GROUND-JUMP_HEIGHT;
        boxes[i].width = 32;
        boxes[i].height = 32;
        boxes[i].mushroom.active = false;
        boxes[i].mushroom.id = i+1;
        sos_vram_load_grande_chunk(0x10 + boxes[i].id, ((uint8_t*) question_block));
        sos_oam_set(boxes[i].id, true, 0x03, false, false, boxes[i].x, boxes[i].y);
        curr_oam++;
        //initialize mushrooms
        sos_vram_load_grande_chunk(0x10 + boxes[i].mushroom.id, ((uint8_t*) mushroom));
        sos_oam_set(boxes[i].mushroom.id, false, 0x02, false, false, boxes[i].x, boxes[i].y);
        curr_oam++;
    }


    // load player oam
    // p1
    p1.oam_base_id = curr_oam++;
    p1.oam_curr_id = p1.oam_base_id;
    sos_vram_load_grande_chunk(0x10 + (p1.oam_base_id), ((uint8_t*) mario_standing));
    sos_oam_set(p1.oam_curr_id, true, 0x04, false, false, p1.x, p1.y);
    curr_oam++;
    sos_vram_load_grande_chunk(0x10 + (curr_oam), ((uint8_t*) mario_walking));
    sos_oam_set(curr_oam, false, 0x05, false, false, p1.x, p1.y);
    curr_oam++;
    sos_vram_load_grande_chunk(0x10 + (curr_oam), ((uint8_t*) mario_jumping));
    sos_oam_set(curr_oam, false, 0x06, false, false, p1.x, p1.y);

    // bullets
    /* for mushrooms
    for (bullet_t *b = &bullets[0]; b <= &bullets[NUM_BULLETS]; b++) {
        b->oam_id = curr_oam++;
        sos_vram_load_grande_chunk(0x10 + (b->oam_id), ((uint8_t*) duck1));
        sos_oam_set(b->oam_id, false, 0x04, false, false, SCREEN_X_MIN, SCREEN_Y_MIN);
    }*/

    // bg vram/oam
    sos_uart_printf("starting bg load\n");
    sos_vram_load_bg(((uint8_t*) overworld));
    sos_uart_printf("bg load done\n");
    //sos_oam_set(BG_OAM, true, 0x00, false, false, SCREEN_X_MIN, SCREEN_Y_MIN);
    

    // Register callbacks
    int i;
    //for (i = 0; i < NUM_DUCKS; i++) {
        //cb_ids[i] = sos_register_vsync_cb(duck_update, &ducks[i], true);
    //}
    //i++;
    i = 0;
    cb_ids[i++] = sos_register_vsync_cb(get_input, &p1, true);
    //cb_ids[i++] = sos_register_vsync_cb(mushroom_update, &boxes, true);
    cb_ids[i++] = sos_register_vsync_cb(player_update, &p1, true);
    cb_ids[i++] = sos_register_vsync_cb(detect_mario_contact, &p1, true);
    sos_uart_printf("user init done\n");
}

// Called in a loop for the remainder of the game
void sos_user_game_tick() {
    int useless = 0;
    if (game_over) {
        sos_uart_printf("game over\n");
        for (sos_cb_id_t *cb_id = &cb_ids[0]; cb_id <= &cb_ids[NUM_CBS - 1]; cb_id++) {
            sos_disable_vsync_cb(*cb_id);
        }
        while (1) {
            useless = useless;
        }
    }
}


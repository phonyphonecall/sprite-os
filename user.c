#include "user.h"
#include "duck1.h"
#include "duck2.h"
#include "duck3.h"
#include "pong_paddle.h"
#include "duck_bg.h"

#define DUCK_HRZ_1 (0x03)
#define DUCK_HRZ_2 (0x04)
#define DUCK_HRZ_3 (0x05)
#define BG_OAM (0x7A)

#define SCREEN_X_MIN (0)
#define SCREEN_Y_MIN (0)
#define SCREEN_X_MAX (640 - 64 - 1)
#define SCREEN_Y_MAX (480 - 64 - 1)

#define MODE_HUMAN  (0)
#define MODE_AI (1)

#define LEFT_SIDE  (0)
#define RIGHT_SIDE (1)

bool game_over = false;

typedef struct player_t {
    uint8_t mode;
    uint8_t side;
    bool left;
    bool right;
    uint16_t x;
    uint8_t oam_id;
} player_t;

player_t p1 = {
    .mode = MODE_HUMAN,
    .side = LEFT_SIDE,
    .left = false,
    .right = false,
    .x = 80
};

player_t p2 = {
    .mode = MODE_AI,
    .side = RIGHT_SIDE,
    .left = false,
    .right = false,
    .x = (SCREEN_X_MAX - 80 - 64)
};

void get_input(void* data) {
    player_t *player = (player_t*) data;

    sos_input_state_t state;
    sos_input_id_t id = sos_get_input_id(player->side);

    sos_fill_input_state(id, &state);
    player->left = state.left;
    player->right = state.right;

    if (state.left) {
        sos_uart_printf("left pressed\n");
    }
    if (state.right) {
        sos_uart_printf("right pressed\n");
    }
}

void player_update(void* data) {
    player_t *p = (player_t*) data;
    if (p->left && p->right) {
        // do nothing
    } else if (p->left) {
        if (p->x < (SCREEN_X_MAX - 64))
            p->x += 2;
    } else if (p->right) {
        if (p->x > 0)
            p->x -= 2;
    } else {
        // do nothing
    }
    // Update OAM
    sos_oam_update(p->oam_id,
        SOS_OAM_OFFSET_X = (uint16_t) p->x;
    );
}


#define NUM_DUCKS (8)
typedef struct duck_t {
    uint8_t id;
    bool alive;
    uint8_t oam_id_horiz_1;
    uint8_t oam_id_horiz_2;
    uint8_t oam_id_horiz_3;
    short x;
    short y;
    short dx;
    short dy;
    uint8_t state;
    uint8_t stall;
} duck_t;
duck_t ducks[NUM_DUCKS];

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

void duck_update(void* data) {
    duck_t *duck = (duck_t*) data;
    if (duck->alive) {
        if (duck->stall % 4 == 0) {
            duck->x += duck->dx;
            duck->state++;
            if (duck->state > 2) {
                duck->state = 0;
            }

            if ((duck->x > (SCREEN_X_MAX - 64)) || (duck->x < 0)) {
                sos_uart_printf("hit end of screen\n");
                duck->dx *= -1;
                duck->x += duck->dx;
            }

            sos_oam_update(duck->oam_id_horiz_1,
                SOS_OAM_ENABLE = (bool) (duck->state == 0);
                SOS_OAM_OFFSET_X = (uint16_t) duck->x;
                SOS_OAM_OFFSET_Y = (uint16_t) duck->y;
                SOS_OAM_FLIP_X = (bool) (duck->dx < 0);
            );
            sos_oam_update(duck->oam_id_horiz_2,
                SOS_OAM_ENABLE = (bool) (duck->state == 1);
                SOS_OAM_OFFSET_X = (uint16_t) duck->x;
                SOS_OAM_OFFSET_Y = (uint16_t) duck->y;
                SOS_OAM_FLIP_X = (bool) (duck->dx < 0);
            );
            sos_oam_update(duck->oam_id_horiz_3,
                SOS_OAM_ENABLE = (bool) (duck->state == 2);
                SOS_OAM_OFFSET_X = (uint16_t) duck->x;
                SOS_OAM_OFFSET_Y = (uint16_t) duck->y - 10;
                SOS_OAM_FLIP_X = (bool) (duck->dx < 0);
            );
        }
        duck->stall++;
    } else {
        // Should spawn? (x/(2^32) times, will spawn)
        if (rand_trigger(8000000)) {
            sos_uart_printf("spawning duck\n");
            duck->alive = true;
            uint32_t r = rand();
            duck->x = 10 + (r % 400);
            duck->dx = 4;
            duck->y = 80 + (r % 250);
            duck->stall = 0;
        }
    }
}


#define KEEP_SIGN(num, sign) (num * ((sign < 0) ? -1 : 1))
#define INCLUSIVE_BETWEEN(diff, low, high) ((diff >= low) ? (diff <= high) : false)

#define NUM_CBS 8
sos_cb_id_t cb_ids[NUM_CBS];

// Register interupts, init graphics etc...
void sos_user_game_init() {
    // load colors
    sos_cram_load_palette(0x03, duck_bg_palette);
    sos_cram_load_palette(0x04, duck1_palette);
    sos_cram_load_palette(0x05, duck2_palette);
    sos_cram_load_palette(0x06, duck3_palette);


    // load duck vram/oam & init duckstruct
    for (uint8_t i = 0; i < NUM_DUCKS; i++) {
        ducks[i].id = i;
        ducks[i].alive = false;
        ducks[i].oam_id_horiz_1 = DUCK_HRZ_1 + (3 * i);
        ducks[i].oam_id_horiz_2 = DUCK_HRZ_2 + (3 * i);
        ducks[i].oam_id_horiz_3 = DUCK_HRZ_3 + (3 * i);
        sos_vram_load_grande_chunk(0x10 + ducks[i].oam_id_horiz_1,
                        ((uint8_t*) duck1));
        sos_vram_load_grande_chunk(0x10 + ducks[i].oam_id_horiz_2,
                        ((uint8_t*) duck2));
        sos_vram_load_grande_chunk(0x10 + ducks[i].oam_id_horiz_3,
                        ((uint8_t*) duck3));
        sos_oam_set(ducks[i].oam_id_horiz_1,
                        false, 0x04, false, false, SCREEN_X_MIN, 80);
        sos_oam_set(ducks[i].oam_id_horiz_2,
                        false, 0x05, false, false, SCREEN_X_MIN, 80);
        // offset 10px up to align duck head
        sos_oam_set(ducks[i].oam_id_horiz_3,
                        false, 0x06, false, false, SCREEN_X_MIN, 80 - 10);
    }

    // load player oam
    // p1
    p1.oam_id = (NUM_DUCKS * 3 + 1);
    sos_vram_load_grande_chunk(0x10 + (p1.oam_id), ((uint8_t*) duck1));
    sos_oam_set(p1.oam_id, true, 0x04, false, false, p1.x, (SCREEN_Y_MAX - 64));
    // p2
    p2.oam_id = p1.oam_id + 1;
    sos_vram_load_grande_chunk(0x10 + (p2.oam_id), ((uint8_t*) duck1));
    sos_oam_set(p2.oam_id, true, 0x04, false, false, p2.x, (SCREEN_Y_MAX - 64));

    // bg vram/oam
    sos_uart_printf("starting bg load\n");
    sos_vram_load_bg(((uint8_t*) duck_bg));
    sos_uart_printf("bg load done\n");
    sos_oam_set(BG_OAM, true, 0x03, false, false, SCREEN_X_MIN, SCREEN_Y_MIN);


    // Register callbacks
    int i;
    for (i = 0; i < NUM_DUCKS; i++) {
        cb_ids[i] = sos_register_vsync_cb(duck_update, &ducks[i], true);
    }
    i++;
    cb_ids[i] = sos_register_vsync_cb(get_input, &p1, true);
    i++;
    cb_ids[i] = sos_register_vsync_cb(get_input, &p2, true);
    i++;
    cb_ids[i] = sos_register_vsync_cb(player_update, &p1, true);
    i++;
    cb_ids[i] = sos_register_vsync_cb(player_update, &p2, true);
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


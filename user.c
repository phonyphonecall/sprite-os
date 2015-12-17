#include "user.h"
#include "duck1.h"
#include "duck2.h"
#include "duck3.h"
#include "bullet.h"
#include "explosion1.h"
#include "explosion2.h"
#include "explosion3.h"
#include "explosion4.h"
#include "explosion5.h"
#include "explosion6.h"
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

#define PLAYER_Y (SCREEN_Y_MAX - 64)

#define MODE_HUMAN  (0)
#define MODE_AI (1)

#define LEFT_SIDE  (0)
#define RIGHT_SIDE (1)

bool game_over = false;

#define NUM_EXPLOSIONS (6)
typedef struct explosion_t {
    bool live;
    uint16_t x;
    uint16_t y;
    uint8_t oam_id;
    uint8_t live_count;
} explosion_t;
explosion_t explosions[NUM_EXPLOSIONS];

void explosion_update(void* data) {
    int i = 0;
    for (explosion_t *e = &explosions[0]; e < &explosions[NUM_EXPLOSIONS]; e++) {
        if (e->live) {
            if (e->live_count > 0) {
                // wait
                sos_oam_update(e->oam_id,
                    SOS_OAM_ENABLE = true;
                    SOS_OAM_OFFSET_X = (uint16_t) e->x;
                    SOS_OAM_OFFSET_Y = (uint16_t) e->y;
                );
                e->live_count--;
            } else {
                e->live = false;
                sos_oam_update(e->oam_id,
                    SOS_OAM_ENABLE = false;
                );
                if (i < 5) {
                    explosion_t *next_exp = &explosions[i+1];
                    next_exp->live = true;
                    next_exp->live_count = 10;
                    next_exp->x = e->x;
                    next_exp->y = e->y;
                    sos_oam_update(next_exp->oam_id,
                        SOS_OAM_ENABLE = true;
                        SOS_OAM_OFFSET_X = (uint16_t) e->x;
                        SOS_OAM_OFFSET_Y = (uint16_t) e->y;
                    );
                }
            }
        }
        i++;
    }
}

#define NUM_BULLETS (10)
#define BULLET_SPEED (3)
#define FIRE_COOLDOWN (40)
typedef struct bullet_t {
    bool live;
    uint16_t x;
    uint16_t y;
    uint8_t oam_id;
} bullet_t;

bullet_t bullets[NUM_BULLETS];
void bullet_update(void* data) {
    for (bullet_t *b = &bullets[0]; b < &bullets[NUM_BULLETS]; b++) {
        if (b->live) {
            b->y -= BULLET_SPEED;
            if (b->y > SCREEN_Y_MIN) {
                sos_oam_update(b->oam_id,
                    SOS_OAM_OFFSET_Y = (uint16_t) b->y;
                );
            } else {
                b->live = false;
                sos_oam_update(b->oam_id,
                    SOS_OAM_ENABLE = false;
                );
            }
        }
    }
}


typedef struct player_t {
    uint8_t mode;
    uint8_t side;
    bool left;
    bool right;
    bool fire;
    uint8_t fire_cooldown;
    uint16_t x;
    uint8_t oam_id;
} player_t;

player_t p1 = {
    .mode = MODE_HUMAN,
    .side = LEFT_SIDE,
    .left = false,
    .right = false,
    .fire = false,
    .fire_cooldown = 0,
    .x = 80
};

player_t p2 = {
    .mode = MODE_AI,
    .side = RIGHT_SIDE,
    .left = false,
    .right = false,
    .fire = false,
    .fire_cooldown = 0,
    .x = (SCREEN_X_MAX - 80 - 64)
};

void get_input(void* data) {
    player_t *player = (player_t*) data;

    sos_input_state_t state;
    sos_input_id_t id = sos_get_input_id(player->side);

    sos_fill_input_state(id, &state);
    player->left = state.left;
    player->right = state.right;
    player->fire = state.up;

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
        if (p->x < SCREEN_X_MAX)
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

    // Fire bullet?
    if (!p->fire_cooldown) {
        if (p->fire) {
            p->fire_cooldown = FIRE_COOLDOWN;
            for (bullet_t *b = &bullets[0]; b < &bullets[NUM_BULLETS]; b++) {
                if (!b->live) {
                    b->live = true;
                    b->x = p->x;
                    b->y = PLAYER_Y;
                    sos_oam_update(b->oam_id,
                        SOS_OAM_ENABLE = true;
                        SOS_OAM_OFFSET_X = (uint16_t) b->x;
                        SOS_OAM_OFFSET_Y = (uint16_t) b->y;
                    );
                    return;
                }
            }
            sos_uart_printf("out of bullets\n");
        }
    } else {
        p->fire_cooldown--;
    }
}


#define NUM_DUCKS (6)
typedef struct duck_t {
    uint8_t id;
    bool alive;
    uint8_t kill_cooldown;
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

            if ((duck->x > SCREEN_X_MAX) || (duck->x < 0)) {
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
        if (!duck->kill_cooldown) {
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
        } else {
            duck->kill_cooldown--;
            if (duck->kill_cooldown == 0) {
                sos_oam_update(duck->oam_id_horiz_1,
                    SOS_OAM_ENABLE = false;
                );
                sos_oam_update(duck->oam_id_horiz_2,
                    SOS_OAM_ENABLE = false;
                );
                sos_oam_update(duck->oam_id_horiz_3,
                    SOS_OAM_ENABLE = false;
                );
            }
        }
    }
}

#define INCLUSIVE_BETWEEN(diff, low, high) ((diff >= low) ? (diff <= high) : false)

#define DUCK_KILL_COOLDOWN (1)
void detect_hits(void* data) {
    for (bullet_t *b = &bullets[0]; b < &bullets[NUM_BULLETS]; b++) {
        for (duck_t *d = &ducks[0]; d < &ducks[NUM_DUCKS]; d++) {
            if (d->alive) {
                if (INCLUSIVE_BETWEEN(b->x, d->x, d->x + 31) &&
                    INCLUSIVE_BETWEEN(b->y, d->y, d->y + 31))
                {
                    d->alive = false;
                    d->kill_cooldown = DUCK_KILL_COOLDOWN;
                    explosions[0].live_count = 10;
                    explosions[0].live = true;
                    explosions[0].x = d->x;
                    explosions[0].y = d->y;
                }
            }
        }
    }
}


#define KEEP_SIGN(num, sign) (num * ((sign < 0) ? -1 : 1))

#define NUM_CBS 8
sos_cb_id_t cb_ids[NUM_CBS];

// Register interupts, init graphics etc...
void sos_user_game_init() {
    // load colors
    sos_cram_load_palette(0x00, duck_bg_palette);
    sos_cram_load_palette(0x04, duck1_palette);
    sos_cram_load_palette(0x05, duck2_palette);
    sos_cram_load_palette(0x06, duck3_palette);
    sos_cram_load_palette(0x07, bullet_palette);
    sos_cram_load_palette(0x08, explosion1_palette);
    sos_cram_load_palette(0x09, explosion2_palette);
    sos_cram_load_palette(0x0A, explosion3_palette);
    sos_cram_load_palette(0x0B, explosion4_palette);
    sos_cram_load_palette(0x0C, explosion5_palette);
    sos_cram_load_palette(0x0D, explosion6_palette);

    // load duck vram/oam & init duckstruct
    uint8_t curr_oam = DUCK_HRZ_1;
    for (uint8_t i = 0; i < NUM_DUCKS; i++) {
        ducks[i].id = i;
        ducks[i].alive = false;
        ducks[i].oam_id_horiz_1 = curr_oam++;
        ducks[i].oam_id_horiz_2 = curr_oam++;
        ducks[i].oam_id_horiz_3 = curr_oam++;
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
    for (int i = 0; i < 6; i++) {
        explosions[i].live = false;
        explosions[i].oam_id = curr_oam++;
    }
    sos_vram_load_grande_chunk(0x10 + explosions[0].oam_id,
                    ((uint8_t*) explosion1));
    sos_vram_load_grande_chunk(0x10 + explosions[1].oam_id,
                    ((uint8_t*) explosion2));
    sos_vram_load_grande_chunk(0x10 + explosions[2].oam_id,
                    ((uint8_t*) explosion3));
    sos_vram_load_grande_chunk(0x10 + explosions[3].oam_id,
                    ((uint8_t*) explosion4));
    sos_vram_load_grande_chunk(0x10 + explosions[4].oam_id,
                    ((uint8_t*) explosion5));
    sos_vram_load_grande_chunk(0x10 + explosions[5].oam_id,
                    ((uint8_t*) explosion6));
    for (int i = 0; i < 6; i++) {
        sos_oam_set(explosions[i].oam_id,
                        false, 0x08 + i, false, false, 0, 0);
    }

    // load player oam
    // p1
    p1.oam_id = curr_oam++;
    sos_vram_load_grande_chunk(0x10 + (p1.oam_id), ((uint8_t*) duck1));
    sos_oam_set(p1.oam_id, true, 0x04, false, false, p1.x, PLAYER_Y);
    // p2
    p2.oam_id = curr_oam++;
    sos_vram_load_grande_chunk(0x10 + (p2.oam_id), ((uint8_t*) duck1));
    sos_oam_set(p2.oam_id, true, 0x04, false, false, p2.x, PLAYER_Y);

    // bullets
    for (bullet_t *b = &bullets[0]; b < &bullets[NUM_BULLETS]; b++) {
        b->oam_id = curr_oam++;
        sos_vram_load_grande_chunk(0x10 + (b->oam_id), ((uint8_t*) bullet));
        sos_oam_set(b->oam_id, false, 0x07, false, false, SCREEN_X_MIN, SCREEN_Y_MIN);
    }

    // bg vram/oam
    sos_uart_printf("starting bg load\n");
    sos_vram_load_bg(((uint8_t*) duck_bg));
    sos_uart_printf("bg load done\n");
    sos_oam_set(BG_OAM, true, 0x00, false, false, SCREEN_X_MIN, SCREEN_Y_MIN);


    // Register callbacks
    int i;
    for (i = 0; i < NUM_DUCKS; i++) {
        cb_ids[i] = sos_register_vsync_cb(duck_update, &ducks[i], true);
    }
    i++;
    cb_ids[i++] = sos_register_vsync_cb(get_input, &p1, true);
    cb_ids[i++] = sos_register_vsync_cb(get_input, &p2, true);
    cb_ids[i++] = sos_register_vsync_cb(player_update, &p1, true);
    cb_ids[i++] = sos_register_vsync_cb(player_update, &p2, true);
    cb_ids[i++] = sos_register_vsync_cb(explosion_update, 0, true);
    cb_ids[i++] = sos_register_vsync_cb(bullet_update, 0, true);
    cb_ids[i++] = sos_register_vsync_cb(detect_hits, 0, true);
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


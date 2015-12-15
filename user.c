#include "user.h"
#include "arrow.h"

#define BG_OAM 0xF0

#define SCREEN_X_MIN 0
#define SCREEN_Y_MIN 0
#define SCREEN_X_MAX (640 - 64 - 1)
#define SCREEN_Y_MAX (480 - 64 - 1)

#define VRAM_INSTANCE_0 0
#define VRAM_SMALL_0 16
#define VRAM_MED_0 66
#define VRAM_LARGE_0 98

void get_input(void* data) {
    player_t *player = (player_t*) data;

    sos_input_state_t state;
    sos_input_id_t id = sos_get_input_id(player->side);

    sos_fill_input_state(id, &state);
    player->left = state.left;
    player->right = state.right;
    player->fire = (state.aux_a | state.aux_b | state.aux_c | state.aux_d);

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
    } else if (p->right) {
        if (p->x < SCREEN_X_MAX)
            p->x += 2;
    } else if (p->left) {
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
            for (bullet_t *b = &bullets[0]; b <= &bullets[NUM_BULLETS]; b++) {
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

// Register interupts, init graphics etc...
void sos_user_game_init() {
    for (int c = 0; c < 16; c++) {
        sos_vram_load_grande_chunk(VRAM_INSTANCE_0 + c, arrow);
    }
    // load colors
    sos_cram_load_palette(0x01, arrow_palette);
}

// Called in a loop for the remainder of the game
void sos_user_game_tick() {
}


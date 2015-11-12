#include "user.h"
#include "pong_ball.h"
#include "pong_paddle.h"

#define BALL_OAM (0x00)
#define PADDLE_1_OAM (0x01)
#define PADDLE_2_OAM (0x02)

#define SCREEN_X_MIN (0)
#define SCREEN_Y_MIN (0)
#define SCREEN_X_MAX (640 - 64 - 1)
#define SCREEN_Y_MAX (480 - 64 - 1)

#define MODE_HUMAN (0)
#define MODE_AI    (1)

#define LEFT_SIDE  (0)
#define RIGHT_SIDE (1)

#define PADDLE_THICKNESS (13)
#define LEFT_PADDLE_SURFACE (PADDLE_THICKNESS)
#define RIGHT_PADDLE_SURFACE (SCREEN_X_MAX - PADDLE_THICKNESS)

bool game_over = false;

typedef struct player_t {
    uint8_t mode;
    uint8_t side;
    short  paddle_y;
    short  paddle_surface;
    bool up;
    bool down;
} player_t;

player_t p1 = {
    .mode = MODE_HUMAN,
    .side = LEFT_SIDE,
    .paddle_y = SCREEN_Y_MIN,
    .paddle_surface = LEFT_PADDLE_SURFACE,
    .up = false,
    .down = false
};

player_t p2 = {
    .mode = MODE_AI,
    .side = RIGHT_SIDE,
    .paddle_y = SCREEN_Y_MIN,
    .paddle_surface = RIGHT_PADDLE_SURFACE,
    .up = false,
    .down = false
};

void get_input(void* data) {
    player_t *player = (player_t*) data;

    sos_input_state_t state;
    sos_input_id_t id = sos_get_input_id(player->side);

    sos_fill_input_state(id, &state);
    player->up = state.up;
    player->down = state.down;

    if (state.up) {
        sos_uart_printf("up pressed\n");
    }
    if (state.down) {
        sos_uart_printf("down pressed\n");
    }
}


short ball_x = 256;
short ball_y = 256;
short ball_dx = 3;
short ball_dy = 3;

void player_human_logic(player_t* player) {
    if (player->up) {
        if (player->paddle_y >= 0) {
            player->paddle_y -= 3;
        }
    } else if (player->down) {
        if (player->paddle_y <= SCREEN_Y_MAX) {
            player->paddle_y += 3;
        }
    }
}

void player_ai_logic(player_t* player) {
    if (player->paddle_y  + 20 > ball_y) {
        player->paddle_y -= 2;
    } else if (player->paddle_y  - 20 < ball_y) {
        player->paddle_y += 2;
    }
}

void player_logic(void* data) {
    player_t *player = (player_t*) data;
    if (player->mode == MODE_AI) {
        player_ai_logic(player);
    } else {
        player_human_logic(player);
    }
}

void paddle_update(void* data) {
    sos_oam_update(PADDLE_1_OAM,
        SOS_OAM_OFFSET_Y = (uint16_t) p1.paddle_y;
    );
    sos_oam_update(PADDLE_2_OAM,
        SOS_OAM_OFFSET_Y = (uint16_t) p2.paddle_y;
    );
}

int frame_count = 0;
int frame_limit = 1;
void ball_update(void* data) {
    frame_count++;
    if (frame_limit == frame_count) {
        frame_count = 0;
    } else {
        return;
    }
    // if (ball_x >= 575 || ball_x <= 0) {
    //     // Move faster, every other hit
    //     if (ball_dx > 0) {
    //         ball_dx += (ball_dx > 0) ? 1 : -1;
    //     }

    //     sos_uart_printf("ball x dir change\n");
    // }
    if (ball_y >= 415 || ball_y <= 0) {
        ball_dy *= -1;
        sos_uart_printf("ball y dir change\n");
    }
    ball_x += ball_dx;
    ball_y += ball_dy;

    sos_oam_update(BALL_OAM,
        SOS_OAM_OFFSET_X = (uint16_t) ball_x;
        SOS_OAM_OFFSET_Y = (uint16_t) ball_y;
    );
}


#define KEEP_SIGN(num, sign) (num * ((sign < 0) ? -1 : 1))
#define INCLUSIVE_BETWEEN(diff, low, high) ((diff >= low) ? (diff <= high) : false)

void check_hit(void* data) {
    player_t *player = (player_t*) data;
    // At either side
    if (((player->side == LEFT_SIDE)  && (ball_x <= player->paddle_surface)) ||
        ((player->side == RIGHT_SIDE) && (ball_x >= player->paddle_surface))) {
        short dy = 0;
        short diff = ball_y - player->paddle_y;
        if (INCLUSIVE_BETWEEN(diff, 26, 64)) {
            dy = 3;
        } else if (INCLUSIVE_BETWEEN(diff, 6, 26)) {
            dy = 2;
        } else if (INCLUSIVE_BETWEEN(diff, -5, 5)) {
            dy = 1;
        } else if (INCLUSIVE_BETWEEN(diff, -26, -6)) {
            dy = 2;
        } else if (INCLUSIVE_BETWEEN(diff, -64, -26)) {
            dy = 3;
        } else {
            if (player->side == LEFT_SIDE) {
                sos_uart_printf("right side wins\n");
            } else {
                sos_uart_printf("left side wins\n");
            }
            game_over = true;
            return;
        }
        ball_dy = KEEP_SIGN(dy, ball_dy);
        // reverse ball direction
        ball_dx *= -1;
        sos_uart_printf("good hit\n");
    }
}


#define NUM_CBS 8
sos_cb_id_t cb_ids[NUM_CBS];

// Register interupts, init graphics etc...
void sos_user_game_init() {
    sos_vram_load_grande_chunk(0x10, ((uint8_t*) pong_ball));
    sos_vram_load_grande_chunk(0x10 + PADDLE_1_OAM, ((uint8_t*) pong_paddle));
    sos_vram_load_grande_chunk(0x10 + PADDLE_2_OAM, ((uint8_t*) pong_paddle));

    sos_cram_load_palette(0x01, pong_ball_palette);
    sos_cram_load_palette(0x02, pong_paddle_palette);

    sos_oam_set(BALL_OAM, true, 0x01, false, false, 256, 256);
    sos_oam_set(PADDLE_1_OAM, true, 0x02, false, true, SCREEN_X_MIN, (uint16_t) p1.paddle_y);
    sos_oam_set(PADDLE_2_OAM, true, 0x02, false, false, SCREEN_X_MAX, (uint16_t)  p2.paddle_y);

    // Register callbacks
    cb_ids[0] = sos_register_vsync_cb(get_input, &p1, true);
    cb_ids[1] = sos_register_vsync_cb(get_input, &p2, true);
    cb_ids[2] = sos_register_vsync_cb(ball_update, 0, true);
    cb_ids[3] = sos_register_vsync_cb(player_logic, &p1, true);
    cb_ids[4] = sos_register_vsync_cb(player_logic, &p2, true);
    cb_ids[5] = sos_register_vsync_cb(paddle_update, 0, true);
    cb_ids[6] = sos_register_vsync_cb(check_hit, &p1, true);
    cb_ids[7] = sos_register_vsync_cb(check_hit, &p2, true);
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


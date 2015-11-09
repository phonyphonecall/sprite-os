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
#define MODE_AI    (0)

#define LEFT_SIDE  (0)
#define RIGHT_SIDE (1)

typedef struct player_t {
    uint8_t side;
    uint8_t mode;
    short  paddle_y;
} player_t;

player_t p1 = {
    .side = LEFT_SIDE,
    .mode = MODE_AI,
    .paddle_y = SCREEN_Y_MIN
};

player_t p2 = {
    .side = RIGHT_SIDE,
    .mode = MODE_AI,
    .paddle_y = SCREEN_Y_MIN
};


short ball_dir_x = 1;
short ball_dir_y = 1;
short ball_x = 1;
short ball_y = 1;

void player_human_logic(player_t* player) {
    // TODO
}

void player_ai_logic(player_t* player) {
    player->paddle_y = ball_y;
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

void ball_update(void* data) {
    if (ball_x >= 575 || ball_x <= 0) {
        ball_dir_x *= -1;
        sos_uart_printf("ball x dir change\n");
    }
    if (ball_y >= 415 || ball_y <= 0) {
        ball_dir_y *= -1;
        sos_uart_printf("ball y dir change\n");
    }
    ball_x += ball_dir_x;
    ball_y += ball_dir_y;

    sos_oam_update(BALL_OAM,
        SOS_OAM_OFFSET_X = (uint16_t) ball_x;
        SOS_OAM_OFFSET_Y = (uint16_t) ball_y;
    );
}

// Register interupts, init graphics etc...
void sos_user_game_init() {
    sos_vram_load_grande_chunk(0x10, ((uint8_t*) pong_ball));
    sos_vram_load_grande_chunk(0x10 + PADDLE_1_OAM, ((uint8_t*) pong_paddle));
    sos_vram_load_grande_chunk(0x10 + PADDLE_2_OAM, ((uint8_t*) pong_paddle));

    sos_cram_load_palette(0x01, pong_ball_palette);
    sos_cram_load_palette(0x02, pong_paddle_palette);

    sos_oam_set(BALL_OAM, true, 0x01, false, false, 0, 0);
    sos_oam_set(PADDLE_1_OAM, true, 0x02, false, true, SCREEN_X_MIN, (uint16_t) p1.paddle_y);
    sos_oam_set(PADDLE_2_OAM, true, 0x02, false, false, SCREEN_X_MAX, (uint16_t)  p2.paddle_y);

    // Register callbacks
    sos_register_vsync_cb(ball_update, 0, true);
    sos_register_vsync_cb(player_logic, &p1, true);
    sos_register_vsync_cb(player_logic, &p2, true);
    sos_register_vsync_cb(paddle_update, 0, true);
    sos_uart_printf("user init done\n");
}

// Called in a loop for the remainder of the game
void sos_user_game_tick() {

}


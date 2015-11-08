#include "user.h"
#include "pong_ball.h"

short ball_dir_x = 1;
short ball_dir_y = 1;
short ball_x = 1;
short ball_y = 1;

void my_isr(void* data) {
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

    sos_oam_update(0x00,
        SOS_OAM_OFFSET_X = (uint16_t) ball_x;
        SOS_OAM_OFFSET_Y = (uint16_t) ball_y;
    );
}

// Register interupts, init graphics etc...
void sos_user_game_init() {
    sos_vram_load_grande_chunk(0x10, ((uint8_t*) pong_ball));

    sos_cram_load_palette(0x01, pong_ball_palette);

    sos_oam_set(0x00, true, 0x01, false, false, 0, 0);
    sos_register_vsync_cb(my_isr, 0, true);
    sos_uart_printf("user init done\n");
}

// Called in a loop for the remainder of the game
void sos_user_game_tick() {

}


#include "graphics.h"
#include "uart.h"
#include "interrupt.h"
#include "user.h"

int main(int argc, char** argv) {
    int i = 2;
    sos_uart_printf("Initializing Sprite-OS\n");
    _sos_init_oam_queue();
    sos_uart_printf("Init done\n");

    sos_user_game_init();

    sos_enable_vsync_interrupt();

    while (1) {
        sos_user_game_tick();
    }
    return 0;
}


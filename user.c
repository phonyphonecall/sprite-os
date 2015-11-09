#include "user.h"

// Register interupts, init graphics etc...
void sos_user_game_init() {
    sos_uart_printf("user init done\n");
}

// Called in a loop for the remainder of the game
void sos_user_game_tick() {
    sos_uart_printf("carter is basic\n");
}


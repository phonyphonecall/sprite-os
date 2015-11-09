#include "uart.h"

void sos_uart_printf(char *string) {
    static char* std_out_map = (char*) 0x84000000 + 0x0004;

    while (*string != '\0') {
        *std_out_map = *string;
        string++;
    }
}


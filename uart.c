#include "uart.h"


void uart_printf(char *string) {
    static char* std_out_map = (char*) 0x83e00000 + 0x1000;

    while (*string != '\0') {
        *std_out_map = *string;
        string++;
    }
}


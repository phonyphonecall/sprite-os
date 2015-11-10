#include "uart.h"

#define UART_STATUS_TX_FULL  (0x10000000)
#define UART_STATUS_TX_EMPTY (0x20000000)

void sos_uart_printf(char *string) {
    uint32_t* status_reg = (uint32_t*) 0x84000000 + 0x0008;
    char* tx = (char*) 0x84000000 + 0x0004;

    while (*string != '\0') {

        // wait until uart is ready for char
        uint32_t status = *status_reg;
        if (status & UART_STATUS_TX_FULL) {
            status = *status_reg;
            while  (!(status & UART_STATUS_TX_EMPTY)) {
                status = *status_reg;
            }
        }
        *tx = *string;
        string++;
    }
}


#include "general.h"

void uart_printf(char *string) {
    static char* std_out_map = (char*) 0x83e00000 + 0x1000;

    while (*string != '\0') {
        *std_out_map = *string;
        string++;
    }
}

void my_isr(void* data) {
    uart_printf("hello from my_isr\n");
}

void my_isr2(void* data) {
    if (*((int*) data) == 69)
        uart_printf("hello from my_isr2 69\n");
    else
        uart_printf("hello from bad isr!");
}


int main(int argc, char** argv) {

    int data = 69;

    // make 2 isr's, both disabled
    sos_cb_id_t id = sos_register_vsync_cb(my_isr, 0, false);
    sos_cb_id_t id2 = sos_register_vsync_cb(my_isr2, &data, false);

    // enable system interrupts
    sos_enable_vsync_interrupt();

    // some time later, enable the isr's
    sos_enable_vsync_cb(id);
    sos_enable_vsync_cb(id2);

    int i = 0;
    while (1) {
        i = i;
    }

    return 0;
}

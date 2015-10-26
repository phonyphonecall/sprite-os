#include "sos.h"

void uart_printf(char *string) {
    static char* std_out_map = (char*) 0x83e00000 + 0x1000;

    while (*string != '\0') {
        *std_out_map = *string;
        string++;
    }
}


int main(int argc, char** argv) {
    int i = 0;

    // print 'diff' when any input changes
    sos_input_id_t id; 
    sos_input_state_diff_t diff;
    sos_input_state_t curr[4];
    sos_input_state_t old[4];
    while (1) {
        for (i = 0; i < 4; i++) {
            id = sos_get_input_id(i);
            sos_fill_input_state(id, &curr[i]);
            if (sos_diff_input_states(old[i], curr[i], &diff)) {
                uart_printf("diff\n");
                old[i].up = curr[i].up;
                old[i].down = curr[i].down;
                old[i].right = curr[i].right;
                old[i].left = curr[i].left;
                old[i].aux_a = curr[i].aux_a;
                old[i].aux_b = curr[i].aux_b;
                old[i].aux_c = curr[i].aux_c;
                old[i].aux_d = curr[i].aux_d;
                old[i].active = curr[i].active;
            }
        }
    }

    return 0;
}

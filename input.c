#include "input.h"

#define SOS_INPUT_BASE_ADDR (0xB0000000)
#define SOS_MAX_INPUT_ADDR  (0xB000000C)

sos_input_id_t sos_get_input_id(int index) {
    return (sos_input_id_t) index * 4;    
}

void sos_fill_input_state(sos_input_id_t id, sos_input_state_t *state) {
    uint32_t val = GET_ADDR((SOS_INPUT_BASE_ADDR + id));
    state->left = ((val & 0x01) != 0);
    state->right = ((val & 0x02) != 0);
    state->up = ((val & 0x04) != 0);
    state->down = ((val & 0x08) != 0);
    state->aux_a = ((val & 0x10) != 0);
    state->aux_b = ((val & 0x20) != 0);
    state->aux_c = ((val & 0x40) != 0);
    state->aux_d = ((val & 0x80) != 0);
    state->active = ((val & 0x100) != 0);
}


bool sos_diff_input_states(sos_input_state_t old, sos_input_state_t new, sos_input_state_diff_t *diff) {
    diff->up = 0;
    diff->down = 0;
    diff->left = 0;
    diff->right = 0;
    diff->aux_a = 0;
    diff->aux_b = 0;
    diff->aux_c = 0;
    diff->aux_d = 0;

    bool is_diff = false;
    if (old.up != new.up) {
        is_diff = true;
        diff->up = (old.up) ? -1 : 1;
    }
    if (old.down != new.down) {
        is_diff = true;
        diff->down = (old.down) ? -1 : 1;
    }
    if (old.left != new.left) {
        is_diff = true;
        diff->left = (old.left) ? -1 : 1;
    }
    if (old.right != new.right) {
        is_diff = true;
        diff->right = (old.right) ? -1 : 1;
    }
    if (old.aux_a != new.aux_a) {
        is_diff = true;
        diff->aux_a = (old.aux_a) ? -1 : 1;
    }
    if (old.aux_b != new.aux_b) {
        is_diff = true;
        diff->aux_b = (old.aux_b) ? -1 : 1;
    }
    if (old.aux_c != new.aux_c) {
        is_diff = true;
        diff->aux_c = (old.aux_c) ? -1 : 1;
    }
    if (old.aux_d != new.aux_d) {
        is_diff = true;
        diff->aux_d = (old.aux_d) ? -1 : 1;
    }

    return is_diff;
}


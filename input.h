#ifndef INPUT_H
#define INPUT_H

#include "sos.h"

typedef uint32_t sos_input_id_t;

typedef struct sos_input_state_t {
    bool up;
    bool down;
    bool left;
    bool right;
    bool aux_a;
    bool aux_b;
    bool aux_c;
    bool aux_d;
    bool active;
} sos_input_state_t;

typedef struct sos_input_state_diff_t {
    int up;
    int down;
    int left;
    int right;
    int aux_a;
    int aux_b;
    int aux_c;
    int aux_d;
    int active;
} sos_input_state_diff_t;

sos_input_id_t sos_get_input_id(int index);
void sos_fill_input_state(sos_input_id_t id, sos_input_state_t *state);
bool sos_diff_input_states(sos_input_state_t a, sos_input_state_t b, sos_input_state_diff_t *diff);

#endif // INPUT_H

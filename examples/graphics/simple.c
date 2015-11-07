#include "sos.h"
#include "gradient.h"

// Display 4 gradient sprites along the right-down diagonal
int main(int argc, char** argv) {
    int i = 0;

    sos_vram_load_grande_chunk(0x10, ((uint8_t*) gradient));
    sos_vram_load_grande_chunk(0x11, ((uint8_t*) gradient));
    sos_vram_load_grande_chunk(0x12, ((uint8_t*) gradient));
    sos_vram_load_grande_chunk(0x13, ((uint8_t*) gradient));

    sos_cram_load_palette(0x02, gradient_palette);

    sos_oam_set(0x00, true, 0x02, false, false, 0, 0);
    sos_oam_set(0x01, true, 0x02, false, false, 128, 128);
    sos_oam_set(0x02, true, 0x02, false, false, 256, 256);
    sos_oam_set(0x03, true, 0x02, false, false, 400, 400);

    while (1) {
        i = i;
    }
}

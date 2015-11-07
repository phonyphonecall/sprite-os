#include "sos.h"
#include "gradient.h"

uint16_t gx = 0;
uint16_t gy = 0;

void my_isr(void* data) {
    sos_oam_update(0x00,
        SOS_OAM_OFFSET_X = gx;
        SOS_OAM_OFFSET_Y = gy;
    );
    gx++;
    gy++;
}

// Display 4 gradient sprites along the right-down diagonal
// translate sprite 0 down the diagonal, 1px per vsync
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


    sos_register_vsync_cb(my_isr, 0, true);
    sos_enable_vsync_interrupt();

    while (1) {
        i = i;
    }
}

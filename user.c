#include "user.h"
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

// Register interupts, init graphics etc...
void sos_user_game_init() {
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
}

// Called in a loop for the remainder of the game
void sos_user_game_tick() {

}


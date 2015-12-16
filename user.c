#include "user.h"
#include "graphics.h"
#include "interrupt.h"
#include "uart.h"
#include "arrow.h"
//#include "track.h"

#define BG_OAM 0xF0

#define SCREEN_X_MIN 0
#define SCREEN_Y_MIN 0
#define SCREEN_X_MAX (640 - 64 - 1)
#define SCREEN_Y_MAX (480 - 64 - 1)

#define VRAM_INSTANCE_0 0
#define VRAM_SMALL_0 16
#define VRAM_MED_0 66
#define VRAM_LARGE_0 98

// int frameCount;
// Track tracks[4];

// void get_input(void* data) {
// }

// void update(void* data) {
//     frameCount++;
//     update_track(&tracks[0], frameCount % 48 == 0);
//     update_track(&tracks[1], frameCount % 64 == 16);
// }

// Register interupts, init graphics etc...
void sos_user_game_init() {
    //frameCount = 0;
    //init_track(&tracks[0], true, true, false, 5, 0x01, 0);
    //init_track(&tracks[1], false, false, true, 5 + 64 + 5, 0x01, 16);

    // load the arrow into all 16 objects and the first mundane
    sos_vram_load_grande_chunk(VRAM_INSTANCE_0, arrow);
    //for (int c = 16; c < 16 + 32; c++) {
    //    sos_vram_load_grande_chunk(VRAM_INSTANCE_0 + c, arrow);
    //}
    // load colors into palette 1
    sos_cram_load_palette(0x01, arrow_palette);
    // set the background color
    //sos_set_default_color(0x000022);

    // show an instance at (500, 200)
    sos_inst_set(0, OBJ_64x64, 0, false, true, 0x01, false, false, 500, 200);
    // show a mundane at (69,20)
    //sos_oam_set(0, true, 0x01, true, false, 15 + 64, 20);
    //sos_register_vsync_cb(update, NULL, true);
    sos_uart_printf("Instance at (500, 200)\n");

}

// Called in a loop for the remainder of the game
void sos_user_game_tick() {
}


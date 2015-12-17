#include "user.h"
#include "graphics.h"
#include "interrupt.h"
#include "uart.h"
#include "arrow.h"
#include "track.h"
#include "song.h"
#include "scott1.h"

#define BG_OAM 0xF0

#define SCREEN_X_MIN 0
#define SCREEN_Y_MIN 0
#define SCREEN_X_MAX (640 - 64 - 1)
#define SCREEN_Y_MAX (480 - 64 - 1)

#define VRAM_INSTANCE_0 0
#define VRAM_SMALL_0 16
#define VRAM_MED_0 66
#define VRAM_LARGE_0 98

 uint32_t frameCount;
 Track tracks[4];

// void get_input(void* data) {
// }

void update(void* data) {
    // frameCount++;
    // bool isBeat = frameCount % 4 == 0;
    // update_track(&tracks[0], isBeat);
    // update_track(&tracks[1], isBeat);
    // update_track(&tracks[2], isBeat);
    // update_track(&tracks[3], isBeat);
    static int oam_entry = 0;
    static int count = 0;
    if (oam_entry + 120 < _NUM_OAM - 2) {
        switch(count) {
            case 0:
                sos_uart_printf("Entry!\n");
                sos_oam_set(120 + oam_entry, true, 0x01, false, false, oam_entry, 0);
                break;
            case 1:
                sos_vram_load_grande_chunk(128 + 4*8 + oam_entry*16, arrow);
                sos_vram_load_grande_chunk(128 + 4*8 + oam_entry*16+5, arrow);
                sos_vram_load_grande_chunk(128 + 4*8 + oam_entry*16+10, arrow);
                sos_vram_load_grande_chunk(128 + 4*8 + oam_entry*16+15, arrow);
                break;
            case 2:
                sos_oam_set(120 + oam_entry, false, 0x00, false, false, 0, 0);
                count = -1;
                oam_entry++;
                break;
        }
        count++;
    }
}

static inline void set_bg(uint32_t x, uint32_t y, uint32_t val) {
    #define BG_CHUNK_START  (192)
    if (x >= 640 || y >= 480) {
        sos_uart_printf("Error: Background point out of bounds!");
        return;
    }

    uint8_t chunk_x = x / 64;
    uint8_t chunk_y = y / 64;
    uint8_t chunk_xPos = x % 64;
    uint8_t chunk_yPos = y % 64;
    uint8_t chunk;
    if (chunk_y == 7) {
        chunk = 70 + chunk_x/2;
        if (chunk_x % 2 == 1) {
            chunk_yPos += 32;
        }
    } else {
        chunk = chunk_y * 10 + chunk_x;
    }

    uint32_t write = ((BG_CHUNK_START + chunk) << 23) |
                     (chunk_yPos << 17) |
                     (chunk_xPos << 11) |
                     (val & 0x0F);
    SET_ADDR(VRAM_BASE_ADDR, write);
}

void init_background() {
    sos_uart_printf("Beginning background setup\n");
    for (int band = 0, offset = 0; band < 12; band++, offset += 40) {
        // draw upper triangle
        for (int diagonal = offset + 38, count = 1; diagonal >= offset; diagonal -= 2, count++) {
            for (int y = 0; y < count; y++) {
                set_bg(diagonal+y, y, band);
                set_bg(diagonal+y+1, y, band);
            }
        }

        // draw intermediate strip
        for (int x = offset - 1, count = 1; x >= 0; x--, count++) {
            for (int y = 0; y < 20; y++) {
                set_bg(x+y, count+y, band);
                set_bg(x+y+1, count+y, band);
            }
        }

        // draw side triangle
        for (int count = 0, max_x = 19; count < 40; count += 2, max_x--) {
            int y = offset + count + 2;
            set_bg(0, y-1, band);
            for (int x = 0; x < max_x; x++) {
                set_bg(x, x+y, band);
                set_bg(x+1, x+y, band);
            }
        }
    }
    sos_uart_printf("Finished background\n");
}

// Register interupts, init graphics etc...
void sos_user_game_init() {
    // frameCount = 0;
    // init_track(&tracks[0], true, false, true, 5 + 69*0, 0x01, 16, song0);
    // init_track(&tracks[1], false, false, false, 5 + 69*1, 0x01, 0, song1);
    // init_track(&tracks[2], false, false, true, 5 + 69*2, 0x01, 32, song2);
    // init_track(&tracks[3], true, false, false, 5 + 69*3, 0x01, 48, song3);

    // load the arrow into all 16 objects and the first mundane
//    sos_vram_load_grande_chunk(VRAM_INSTANCE_0, arrow);
    // load colors into palette 1
    //sos_cram_load_palette(0x00, arrow_palette);
    sos_cram_load_palette(0x01, arrow_palette);
    // #define SCOTT_OFFSET (192-16)
    // for (int y = 0; y < 4; y++) {
    //     for (int x = 0; x < 4; x++) {
    //         for (int row = 0; row < 64; row++) {
    //             for (int col = 0; col < 64; col++) {
    //                 uint32_t write = ((SCOTT_OFFSET + y*4 + x) << 23) |
    //                                  (row << 17) |
    //                                  (col << 11) |
    //                                  (scott1[y*64*4 + x*64 + row*64*4 + col] & 0x0F);
    //                 SET_ADDR(VRAM_BASE_ADDR, write);
    //             }
    //         }
    //     }
    // }

//    sos_cram_load_palette(0x01, arrow_palette);
    // set the background color
    //sos_set_default_color(0x0000CC);

    // show an instance at (500, 200)
//    sos_inst_set(0, OBJ_64x64, 0, false, true, 0x01, false, false, 500, 200);
    // show a mundane at (69,20)
    //sos_oam_set(0, false, 0x01, true, false, 15 + 64, 20);
    sos_register_vsync_cb(update, NULL, true);
    // sos_oam_set(121, true, 0x01, false, false, 300, 100);

    //init_background();
    sos_uart_printf("DDR init done\n");
}

// Called in a loop for the remainder of the game
void sos_user_game_tick() {
}


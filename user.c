#include "user.h"
#include "graphics.h"
#include "interrupt.h"
#include "uart.h"
#include "input.h"

#include "arrow.h"
#include "track.h"
#include "song.h"
#include "scott1.h"
#include "background.h"
#include "receptor.h"

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

void get_input(void* data) {
    sos_input_state_t input;
    sos_fill_input_state(0, &input);
    control_track(&tracks[0], input.left  | input.aux_d);
    control_track(&tracks[1], input.down  | input.aux_b);
    control_track(&tracks[2], input.up    | input.aux_c);
    control_track(&tracks[3], input.right | input.aux_a);
}

void update(void* data) {
    frameCount++;
    bool isBeat = frameCount % VS_PER_TICK == 0;
    update_track(&tracks[0], isBeat);
    update_track(&tracks[1], isBeat);
    update_track(&tracks[2], isBeat);
    update_track(&tracks[3], isBeat);
    if (isBeat) {
        rotate_bg_palette();
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
    int offset = 0;
    // draw left triangle of screen
    for (int band = 0; band < 12; band++, offset += 40) {
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

    // draw center rhombus
    for (int band = 12; band < 16; band++, offset += 40) {
        // draw upper triangle
        for (int diagonal = offset + 38, count = 1; diagonal >= offset; diagonal -= 2, count++) {
            for (int y = 0; y < count; y++) {
                set_bg(diagonal+y, y, band);
                set_bg(diagonal+y+1, y, band);
            }
        }

        // draw intermediate strip
        for (int x = offset - 1, count = 1; count < 480-20; x--, count++) {
            for (int y = 0; y < 20; y++) {
                set_bg(x+y, count+y, band);
                set_bg(x+y+1, count+y, band);
            }
        }

        // draw bottom triangle
        for (int count = offset - 480 + 20, y = -20; y < 0; count--, y++) {
            for (int x = 0; x < -y; x++) {
                set_bg(count+x, 480+x+y, band);
                set_bg(count+x+1, 480+x+y, band);
            }
        }
    }

    // draw right triangle of screen
    for (int band = 0; band < 12; band++, offset += 40) {
        // draw side triangle
        for (int count = 0; count < 20; count++) {
            for (int pos = 0; pos < count; pos++) {
                set_bg(639-count+pos, offset+count+pos-639, band);
                set_bg(640-count+pos, offset+count+pos-639, band);
            }
            set_bg(639, offset-640+count*2+1, band);
        }

        // draw intermediate strip
        for (int x = 640 - 20 - 1, count = offset-x; count < 480-20; x--, count++) {
            for (int y = 0; y < 20; y++) {
                set_bg(x+y, count+y, band);
                set_bg(x+y+1, count+y, band);
            }
        }

        // draw bottom triangle
        for (int count = offset - 480 + 20, y = -20; y < 0; count--, y++) {
            for (int x = 0; x < -y; x++) {
                set_bg(count+x, 480+x+y, band);
                set_bg(count+x+1, 480+x+y, band);
            }
        }
    }

    sos_uart_printf("Finished background\n");
}

void load_dancer() {
    // BORKED!!!!
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
    // sos_oam_set(121, true, 0x01, false, false, 300, 100);
}

// Register interupts, init graphics etc...
void sos_user_game_init() {
    frameCount = 0;
    init_track(&tracks[0], true, false, true, 5 + 69*0, 0x14, 0, song0, 0, true);
    init_track(&tracks[1], false, false, false, 5 + 69*1, 0x15, 16, song1, 1, false);
    init_track(&tracks[2], false, false, true, 5 + 69*2, 0x16, 32, song2, 2, true);
    init_track(&tracks[3], true, false, false, 5 + 69*3, 0x17, 48, song3, 3, false);

    // load the arrow into all 16 objects and the first mundane
    sos_vram_load_grande_chunk(VRAM_INSTANCE_0, arrow);
    // load colors into palette 1
    sos_set_default_color(bg_palette[0]);
    sos_cram_load_palette(0x00, bg_palette+1);
    sos_cram_load_palette(0x10, arrow_palette_0);
    sos_cram_load_palette(0x11, arrow_palette_1);
    sos_cram_load_palette(0x12, arrow_palette_2);
    sos_cram_load_palette(0x13, arrow_palette_3);
    sos_cram_load_palette(0x14, arrow_palette_4);
    sos_cram_load_palette(0x15, arrow_palette_5);
    sos_cram_load_palette(0x16, arrow_palette_6);
    sos_cram_load_palette(0x17, arrow_palette_7);
    sos_cram_load_palette(0x18, arrow_palette_8);
    sos_cram_load_palette(0x19, arrow_palette_9);
    sos_cram_load_palette(0x1A, arrow_palette_10);
    sos_cram_load_palette(0x1B, arrow_palette_11);
    sos_cram_load_palette(0x1C, arrow_palette_12);
    sos_cram_load_palette(0x1D, arrow_palette_13);
    sos_cram_load_palette(0x1E, arrow_palette_14);
    sos_cram_load_palette(0x1F, arrow_palette_15);

    sos_cram_load_palette(RECEPTOR_PALETTE, receptor_palette);
    sos_cram_load_palette(ACTIVE_PALETTE, active_palette);
    sos_cram_load_palette(MISSED_PALETTE, missed_palette);

    load_dancer();

    sos_vram_load_grande_chunk(16, receptor_right);
    sos_vram_load_grande_chunk(17, receptor_down);
    sos_vram_load_grande_chunk(18, receptor_down);
    sos_vram_load_grande_chunk(19, receptor_right);

    //init_background();

    sos_register_vsync_cb(update, NULL, true);
    sos_register_vsync_cb(get_input, NULL, true);

    sos_uart_printf("DDR init done\n");
}

// Called in a loop for the remainder of the game
void sos_user_game_tick() {
}


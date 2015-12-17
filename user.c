#include "user.h"
#include "graphics.h"
#include "interrupt.h"
#include "uart.h"
#include "input.h"

#include "arrow.h"
#include "track.h"
#include "song.h"
#include "scott1.h"
#include "scott2.h"
#include "background.h"
#include "receptor.h"
#include "easy.h"
#include "medium.h"
#include "hard.h"
#include "challenge.h"

#define BG_OAM 0xF0
#define OAM_MED_0 112
#define OAM_LARGE_0 120

#define SCREEN_X_MIN 0
#define SCREEN_Y_MIN 0
#define SCREEN_X_MAX (640 - 64 - 1)
#define SCREEN_Y_MAX (480 - 64 - 1)

 uint32_t frameCount;
 Track tracks[4];
 uint32_t dancer_mode;
 bool level_select;
 bool level_low;
 bool level_high;
 int level;

void do_level_select() {
    // hide receptors
    sos_oam_set(0, false, RECEPTOR_PALETTE,  false, false, 0, SENSOR_Y_POS);
    sos_oam_set(1, false, RECEPTOR_PALETTE,  false, false, 0, SENSOR_Y_POS);
    sos_oam_set(2, false, RECEPTOR_PALETTE,  false, false, 0, SENSOR_Y_POS);
    sos_oam_set(3, false, RECEPTOR_PALETTE,  false, false, 0, SENSOR_Y_POS);

    sos_set_default_color(0);
    sos_cram_load_palette(0, bg_black);
    sos_oam_set(OAM_MED_0, true, EASY_PALETTE, false, false, 32, 128-32);
    sos_oam_set(OAM_MED_0+1, true, MEDIUM_PALETTE, false, false, 640-128-32, 128-32);
    sos_oam_set(OAM_MED_0+2, true, HARD_PALETTE, false, false, 32, 256);
    sos_oam_set(OAM_MED_0+3, true, CHALLENGE_PALETTE, false, false, 640-128-32, 256);
    sos_oam_set(OAM_LARGE_0, false, 0x01, false, false, 300, 100);
    sos_oam_set(OAM_LARGE_0+1, true, 0x02, false, true, 320-128, 100);
    level_select = true;
    level_low = false;
    level_high = false;
}


void start_game() {
    level = (level_high ? 2 : 0) + (level_low ? 1 : 0);
    frameCount = 0;
    init_track(&tracks[0], true, false, true, 5 + 69*0, 0, song0[level], 0, true);
    init_track(&tracks[1], false, false, false, 5 + 69*1, 16, song1[level], 1, false);
    init_track(&tracks[2], false, false, true, 5 + 69*2, 32, song2[level], 2, true);
    init_track(&tracks[3], true, false, false, 5 + 69*3, 48, song3[level], 3, false);

    // hide the menu screen, setup dancer
    sos_oam_set(OAM_MED_0, false, EASY_PALETTE, false, false, 32, 128-32);
    sos_oam_set(OAM_MED_0+1, false, MEDIUM_PALETTE, false, false, 640-128-32, 128-32);
    sos_oam_set(OAM_MED_0+2, false, HARD_PALETTE, false, false, 32, 256);
    sos_oam_set(OAM_MED_0+3, false, CHALLENGE_PALETTE, false, false, 640-128-32, 256);
    sos_oam_set(OAM_LARGE_0, false, 0x01, false, false, 300, 100);
    sos_oam_set(OAM_LARGE_0+1, false, 0x02, false, true, 300, 100);

    level_select = false;
}

void get_input(void* data) {
    sos_input_state_t input;
    sos_fill_input_state(0, &input);
    if (level_select) {
        if (input.aux_a) {
            start_game();
        } else {
            if (input.up) {
                level_high = false;
                sos_oam_update(OAM_LARGE_0 + 1,
                    SOS_OAM_FLIP_Y = false;
                );
            } else if (input.down) {
                level_high = true;
                sos_oam_update(OAM_LARGE_0 + 1,
                    SOS_OAM_FLIP_Y = true;
                );
            }
            if (input.left) {
                level_low = false;
                sos_oam_update(OAM_LARGE_0 + 1,
                    SOS_OAM_FLIP_X = true;
                );
            } else if (input.right) {
                level_low = true;
                sos_oam_update(OAM_LARGE_0 + 1,
                    SOS_OAM_FLIP_X = false;
                );
            }
        }
    } else {
        control_track(&tracks[0], input.left  | input.aux_d);
        control_track(&tracks[1], input.down  | input.aux_b);
        control_track(&tracks[2], input.up    | input.aux_c);
        control_track(&tracks[3], input.right | input.aux_a);
    }
}

void update_dancer(int tickCount) {
    static bool flips[8] = {
        true,
        false,
        true,
        true,
        true,
        false,
        false,
        false
    };
    static bool egyptian[8] = {
        true,
        true,
        true,
        false,
        true,
        true,
        false,
        true
    };
    sos_oam_update(OAM_LARGE_0,
        SOS_OAM_ENABLE=egyptian[tickCount/2];
        SOS_OAM_FLIP_X=flips[tickCount/2];
    );
    sos_oam_update(OAM_LARGE_0+1,
        SOS_OAM_ENABLE=!egyptian[tickCount/2];
        SOS_OAM_FLIP_X=flips[tickCount/2];
    );
}

void update(void* data) {
    if (!level_select) {
        if (frameCount == 0) {
            init_background();
        }
        bool isBeat = frameCount % VS_PER_TICK == 0;
        int tickCount = (frameCount/VS_PER_TICK) % 16;
        bool done = true;
        done &= update_track(&tracks[0], isBeat, tickCount);
        done &= update_track(&tracks[1], isBeat, tickCount);
        done &= update_track(&tracks[2], isBeat, tickCount);
        done &= update_track(&tracks[3], isBeat, tickCount);
        if (isBeat) {
            rotate_bg_palette();
            update_dancer(tickCount);
        }
        frameCount++;

        if (done) {
            do_level_select();
        }
    }
}

void load_dancer() {
    sos_vram_load_venti(0, scott1);
    sos_vram_load_venti(1, scott2);
    sos_cram_load_palette(0x01, scott1_palette);
    sos_cram_load_palette(0x02, scott2_palette);
}

void load_level_select() {
    sos_vram_load_vrende(0, easy);
    sos_vram_load_vrende(1, medium);
    sos_vram_load_vrende(2, hard);
    sos_vram_load_vrende(3, challenge);
    sos_cram_load_palette(EASY_PALETTE, easy_palette);
    sos_cram_load_palette(MEDIUM_PALETTE, medium_palette);
    sos_cram_load_palette(HARD_PALETTE, hard_palette);
    sos_cram_load_palette(CHALLENGE_PALETTE, challenge_palette);
}

void load_arrows() {
    // load the arrow into all objects
    for (int c = 0; c < 16; c++) {
        sos_vram_load_grande_chunk(VRAM_INSTANCE_0+c, arrow);
    }
    // load colors into palette 1
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

    sos_vram_load_grande_chunk(16, receptor_right);
    sos_vram_load_grande_chunk(17, receptor_down);
    sos_vram_load_grande_chunk(18, receptor_down);
    sos_vram_load_grande_chunk(19, receptor_right);
}

// Register interupts, init graphics etc...
void sos_user_game_init() {
    load_dancer();
    load_level_select();
    load_arrows();

    do_level_select();

    sos_register_vsync_cb(update, NULL, true);
    sos_register_vsync_cb(get_input, NULL, true);

    sos_uart_printf("DDR init done\n");
}

// Called in a loop for the remainder of the game
void sos_user_game_tick() {
}


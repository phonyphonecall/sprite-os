uint32_t bg_palette[] = {
0x004040ff,
0x005858ff,
0x006d6dff,
0x007b7bff,
0x008080ff,
0x007b7bff,
0x006d6dff,
0x005858ff,
0x004040ff,
0x002727ff,
0x001313ff,
0x000505ff,
0x000000ff,
0x000505ff,
0x001313ff,
0x002727ff};

uint32_t bg_black[] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

void rotate_bg_palette() {
    uint32_t tmp = bg_palette[0];
    for (int c = 0; c < 15; c++) {
        bg_palette[c] = bg_palette[c+1];
    }
    bg_palette[15] = tmp;

    sos_set_default_color(bg_palette[0]);
    sos_cram_load_palette(0x00, bg_palette+1);
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
    sos_set_default_color(bg_palette[0]);
    sos_cram_load_palette(0x00, bg_palette+1);

    static bool bg_loaded = false;
    if (bg_loaded) return;
    bg_loaded = true;

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

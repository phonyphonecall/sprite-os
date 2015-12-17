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

void rotate_bg_palette() {
    uint32_t tmp = bg_palette[0];
    for (int c = 0; c < 15; c++) {
        bg_palette[c] = bg_palette[c+1];
    }
    bg_palette[15] = tmp;

    sos_set_default_color(bg_palette[0]);
    sos_cram_load_palette(0x00, bg_palette+1);
}

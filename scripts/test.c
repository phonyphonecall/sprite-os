#include <stdio.h>
#include <stdint.h>

int main(int argc, char** argv) {
    struct vram_set {
        unsigned int palette : 4;
        unsigned int _res    : 7;
        unsigned int pixel_x : 6;
        unsigned int pixel_y : 6;
        unsigned int chunk   : 9;
    };
    union vram_converter {
        uint32_t val;
        struct vram_set set;
    };

    struct vram_set set = {
        .palette = 5,
        ._res = 0,
        .pixel_x = 63,
        .pixel_y = 62,
        .chunk = 0x11
    };
    union vram_converter conv = {
        .set = set
    };
    printf("0x%08x\n", conv.val);
}


#include "graphics.h"

#define OAM_BASE_ADDR (0xA0000000)
#define CRAM_BASE_ADDR (0xA0000400)
#define VRAM_BASE_ADDR (0xA0000800)

void sos_vram_load_grande_chunk(uint16_t chunk_num, uint8_t *color_indecies) {
    struct vram_set {
        unsigned int chunk   : 9;
        unsigned int pixel_y : 6;
        unsigned int pixel_x : 6;
        unsigned int _res    : 7;
        unsigned int p_data  : 4;
    };
    union vram_converter {
        uint32_t val;
        struct vram_set set;
    };

    for (unsigned int y = 0; y < GRANDE_HEIGHT; y++) {
        for (unsigned int x = 0; x < GRANDE_WIDTH; x++) {
            uint8_t color = color_indecies[(y * GRANDE_HEIGHT) + x];
            struct vram_set set = {
                .p_data = color,
                ._res = 0,
                .pixel_x = x,
                .pixel_y = y,
                .chunk = chunk_num
            };
            union vram_converter conv = {
                .set = set
            };
            SET_ADDR(VRAM_BASE_ADDR, conv.val);
        }
    }
}





// sos_vram_load_vrende_chunk(uint16_t chunk_num, uint8_t *color_indecies[VRENDE_CHUNK_SIZE]);
// sos_vram_load_venti_chunk(uint16_t chunk_num, uint8_t *color_indecies[VENTI_CHUNK_SIZE]);
// 
void sos_cram_load_palette(uint8_t palette_num, uint32_t *palette) {
    // turn ID into byte offset
    uint32_t cram_offset = palette_num * 4;
    struct cram_set {
        unsigned int offset : 4;
        unsigned int _res   : 4;
        unsigned int r      : 8;
        unsigned int g      : 8;
        unsigned int b      : 8;
    };
    union cram_converter {
        uint32_t val;
        struct cram_set set;
    };

    // Skip palette index 0 always
    for (uint8_t i = 0; i < PALETTE_SIZE; i++) {
        struct cram_set set = {
            .offset = i + 1,
            ._res   = 0,
            .r      = (palette[i] & ((uint32_t) 0x00FF0000)) >> 16,
            .g      = (palette[i] & ((uint32_t) 0x0000FF00)) >> 8,
            .b      = (palette[i] & ((uint32_t) 0x000000FF))
        };
        union cram_converter conv = {
            .set = set
        };
        SET_ADDR((CRAM_BASE_ADDR + cram_offset), conv.val);
    }
}


void sos_oam_set(uint8_t entry_num,
                    bool enable,
                    uint8_t palette_num,
                    bool flip_y,
                    bool flip_x,
                    uint16_t x_offset,
                    uint16_t y_offset)
{
    uint32_t oam_offset = entry_num * 4;
    struct oam_set set = {
        .en       = enable,
        .palette  = palette_num,
        .flip_y   = flip_y,
        .flip_x   = flip_x,
        .x_offset = x_offset,
        .y_offset = y_offset
    };
    union oam_converter conv = {
        .set = set
    };
    SET_ADDR((OAM_BASE_ADDR + oam_offset), conv.val);
}

// 
// void sos_oam_inst_set(uint8_t entry_num,
//                         bool enable,
//                         uint8_t palette_num,
//                         bool flip_y,
//                         bool flip_x,
//                         uint16_t x_offset,
//                         uint16_t y_offset
//                         sprite_size_t size,
//                         uint8_t sprite_index
//                         bool transpose);

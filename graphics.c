#include "graphics.h"
#include "interrupt.h"

#define BG_CHUNK_START  (0x82)
#define BG_CHUNK_END    (0xCD)

// Holds current diff state of oam
_flagged_word _oam_regs[_NUM_OAM];

void sos_vram_load_grande_chunk(uint16_t chunk_num, uint8_t *color_indecies) {
    struct vram_set {
        unsigned int _res_0  : 1;
        unsigned int chunk   : 8;
        unsigned int pixel_y : 6;
        unsigned int pixel_x : 6;
        unsigned int _res_1  : 7;
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
                ._res_0 = 0,
		._res_1 = 0,
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

void sos_vram_load_vrende(uint8_t index, uint8_t *sprite) {
    int offset = (VRAM_MED_0 + 4*index);
    for (int y = 0; y < 2; y++) {
        for (int x = 0; x < 2; x++) {
            for (int row = 0; row < 64; row++) {
                for (int col = 0; col < 64; col++) {
                    uint32_t write = ((offset + y*2 + x) << 23) |
                                     (row << 17) |
                                     (col << 11) |
                                     (sprite[y*64*64*2 + x*64 + row*64*2 + col] & 0x0F);
                    SET_ADDR(VRAM_BASE_ADDR, write);
                }
            }
        }
    }
}

void sos_vram_load_venti(uint8_t index, uint8_t *sprite) {
    int offset = (VRAM_LARGE_0 + 16*index);
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            for (int row = 0; row < 64; row++) {
                for (int col = 0; col < 64; col++) {
                    uint32_t write = ((offset + y*4 + x) << 23) |
                                     (row << 17) |
                                     (col << 11) |
                                     (sprite[y*64*64*4 + x*64 + row*64*4 + col] & 0x0F);
                    SET_ADDR(VRAM_BASE_ADDR, write);
                }
            }
        }
    }
}

void sos_vram_load_bg(uint8_t *color_indecies) {
    for (uint16_t i = 0; i < (BG_CHUNK_END - BG_CHUNK_START); i += 1) {
        sos_vram_load_grande_chunk((i + BG_CHUNK_START), &color_indecies[i * 64 * 64]);
    }
}

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

// OAM QUEUEING
void oam_queue_worker(void* args) {
    for (int i = 0; i < _NUM_OAM; i++) {
        _flagged_word *reg = &_oam_regs[i];
        if (reg->changed == true) {
            SET_ADDR((OAM_BASE_ADDR + (i * 4)), reg->word);
            reg->changed = false;
        }
    }
}

void _sos_init_oam_queue() {
    memset((void*) _oam_regs, 0, sizeof(_oam_regs));
    sos_register_vsync_cb(oam_queue_worker, 0, true);
}

void sos_oam_set(uint16_t entry_num,
                    bool enable,
                    uint8_t palette_num,
                    bool flip_y,
                    bool flip_x,
                    uint16_t x_offset,
                    uint16_t y_offset)
{
    struct _oam_set set = {
        .en       = enable,
        .palette  = palette_num,
        .flip_y   = flip_y,
        .flip_x   = flip_x,
        .x_offset = x_offset,
        .y_offset = y_offset
    };
    union _oam_converter conv = {
        .set = set
    };
    if (entry_num >= _INST_BASE) {
        // instances get directly pushed to memory
        SET_ADDR((OAM_BASE_ADDR + entry_num*4), (conv.val));
    } else {
        _oam_regs[entry_num].changed = true;
        _oam_regs[entry_num].word = conv.val;
    }
}

void sos_inst_obj_set(uint16_t inst_num,
                        uint8_t size,
                        uint8_t sprite_index,
                        bool transpose)
{
    struct _obj_set set = {
        .size = size,
        .sprite_index = sprite_index,
        .transpose = transpose
    };
    union _obj_converter conv = {
        .set = set
    };
    SET_ADDR((OAM_BASE_ADDR + 4*(2*inst_num + _INST_BASE + 1)), (conv.val));
}


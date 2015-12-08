#include "graphics.h"

#define OAM_BASE_ADDR (0xA0000000)
#define CRAM_BASE_ADDR (0xA0000400)
#define VRAM_BASE_ADDR (0xA0000800)

#define BG_CHUNK_START  (0xC0)
#define BG_CHUNK_END    (0x10B)

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

    for (int i = 0; i < _NUM_INST_OAM; i++) {
        _flagged_double_word *reg = &_oam_inst_regs[i];
        if (reg->changed == true) {
            SET_ADDR((OAM_BASE_ADDR + 0x200 + (i * 4)), reg->word1);
            SET_ADDR((OAM_BASE_ADDR + 0x204 + (i * 4)), reg->word2);
            reg->changed = false;
        }
    }
}

void _sos_init_oam_queue() {
    memset((void*) _oam_regs, 0, sizeof(_oam_regs));
    memset((void*) _oam_inst_regs, 0, sizeof(_oam_inst_regs));
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
    _oam_regs[entry_num].changed = true;
    _oam_regs[entry_num].word = conv.val;
}

void sos_oam_inst_set(uint8_t entry_num,
                        bool enable,
                        uint8_t palette_num,
                        bool flip_y,
                        bool flip_x,
                        uint16_t x_offset,
                        uint16_t y_offset,
                        uint8_t size,
                        uint8_t sprite_index,
                        bool transpose)
{
    struct _oam_set set1 = {
        .en       = enable,
        .palette  = palette_num,
        .flip_y   = flip_y,
        .flip_x   = flip_x,
        .x_offset = x_offset,
        .y_offset = y_offset
    };
    union _oam_converter conv1 = {
        .set = set1
    };
    struct _inst_set set2 = {
        .size = size,
        .sprite_index = sprite_index,
        .transpose = transpose
    };
    union _inst_converter conv2 = {
        .set = set2
    };
    _oam_inst_regs[entry_num].changed = true;
    _oam_inst_regs[entry_num].word1 = conv1.val;
    _oam_inst_regs[entry_num].word2 = conv2.val;
}


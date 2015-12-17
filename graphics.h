#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "sos.h"

#define GRANDE_WIDTH (64)
#define GRANDE_HEIGHT (64)
#define GRANDE_CHUNK_SIZE (4096)
#define VRENDE_CHUNK_SIZE (16384)
#define VENTI_CHUNK_SIZE (65536)

#define PALETTE_SIZE (15)

#define SPRITE_GRANDE (0x0)
#define SPRITE_TALL   (0x1)
#define SPRITE_VRENDE (0x2)
#define SPRITE_VENTI  (0x3)

#define OAM_BASE_ADDR (0xA0000000)
#define CRAM_BASE_ADDR (0xA0000400)
#define VRAM_BASE_ADDR (0xA0000800)

// Private datastructures needed publicly by the update macro
#define _NUM_OAM 124
#define _NUM_INST_OAM 64
struct _oam_set {
    unsigned int en         : 1;
    unsigned int _res0      : 1;
    unsigned int palette    : 6;
    unsigned int _res1      : 2;
    unsigned int flip_y     : 1;
    unsigned int flip_x     : 1;
    unsigned int x_offset   : 10;
    unsigned int _res2      : 1;
    unsigned int y_offset   : 9;
};
union _oam_converter {
    uint32_t val;
    struct _oam_set set;
};

typedef struct _flagged_word {
    bool changed;
    uint32_t word;
} _flagged_word;

typedef struct _flagged_double_word {
    bool changed;
    uint32_t word1;
    uint32_t word2;
} _flagged_double_word;

// Holds current diff state of oam
_flagged_word _oam_regs[_NUM_OAM];
_flagged_double_word _oam_inst_regs[_NUM_INST_OAM];


// Public API's
void sos_vram_load_grande_chunk(uint16_t chunk_num, uint8_t *color_indecies);
// void sos_vram_load_vrende_chunk(uint16_t chunk_num, uint8_t *color_indecies[]);
// void sos_vram_load_venti_chunk(uint16_t chunk_num, uint8_t *color_indecies[]);

void sos_vram_load_bg(uint8_t *color_indecies);

void sos_cram_load_palette(uint8_t palette_num, uint32_t *palette);

void sos_oam_set(uint16_t entry_num,
                    bool enable,
                    uint8_t palette_num,
                    bool flip_y,
                    bool flip_x,
                    uint16_t x_offset,
                    uint16_t y_offset);

// OAM update macros
#define SOS_OAM_ENABLE      conv.set.en
#define SOS_OAM_PALETTE     conv.set.palette
#define SOS_OAM_FLIP_X      conv.set.flip_x
#define SOS_OAM_FLIP_Y      conv.set.flip_y
#define SOS_OAM_OFFSET_X    conv.set.x_offset
#define SOS_OAM_OFFSET_Y    conv.set.y_offset
#define sos_oam_update(entry_num, updates) do {\
        union _oam_converter conv = {\
            .val = _oam_regs[entry_num].word\
        };\
        updates\
        _oam_regs[entry_num].changed = true;\
        _oam_regs[entry_num].word = conv.val;\
    } while (0)


struct _inst_set {
    unsigned int size         : 1;
    unsigned int sprite_index : 1;
    unsigned int transpose    : 6;
};
union _inst_converter {
    uint32_t val;
    struct _inst_set set;
};

void sos_oam_set_inst(uint8_t entry_num,
                        bool enable,
                        uint8_t palette_num,
                        bool flip_y,
                        bool flip_x,
                        uint16_t x_offset,
                        uint16_t y_offset,
                        uint8_t size,
                        uint8_t sprite_index,
                        bool transpose);

// internal functions
void _sos_init_oam_queue(void);

#endif //GRAPHICS_H


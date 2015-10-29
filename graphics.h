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



struct oam_set {
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
union oam_converter {
    uint32_t val;
    struct oam_set set;
};

void sos_vram_load_grande_chunk(uint16_t chunk_num, uint8_t *color_indecies);
void sos_vram_load_vrende_chunk(uint16_t chunk_num, uint8_t *color_indecies[]);
void sos_vram_load_venti_chunk(uint16_t chunk_num, uint8_t *color_indecies[]);

void sos_cram_load_palette(uint8_t palette_num, uint32_t *palette);


void sos_oam_set(uint8_t entry_num,
                    bool enable,
                    uint8_t palette_num,
                    bool flip_y,
                    bool flip_x,
                    uint16_t x_offset,
                    uint16_t y_offset);

// OAM update macros
// ex: update x and y offsets
// SOS_OAM_UPDATE(0x04,
//      SOS_OAM_OFFSET_X = 45;
//      SOS_OAM_OFFSET_Y = 52;
// );
#define SOS_OAM_ENABLE      conv.set.en
#define SOS_OAM_PALETTE     conv.set.palette
#define SOS_OAM_FLIP_X      conv.set.flip_x
#define SOS_OAM_FLIP_Y      conv.set.flip_y
#define SOS_OAM_OFFSET_X    conv.set.x_offset
#define SOS_OAM_OFFSET_Y    conv.set.y_offset

#define SOS_OAM_UPDATE(entry_num, updates) do {\
        uint32_t oam_offset = entry_num * 4;\
        union oam_converter conv = {\
            .val = GET_ADDR((OAM_BASE_ADDR + oam_offset))\
        };\
        updates\
        SET_ADDR((OAM_BASE_ADDR + oam_offset), conv.val);\
    } while (0)

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

#endif //GRAPHICS_H



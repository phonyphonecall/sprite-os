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



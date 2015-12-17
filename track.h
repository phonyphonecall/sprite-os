#ifndef _TRACK_H
#define _TRACK_H

#define NUM_ARROWS_IN_TRACK 16

struct __Track;

typedef struct __Arrow {
    int instIndex;
    int paletteIndex;
    int yPos;
    struct __Track *track;
} Arrow;

typedef struct __Track {
    bool transpose;
    bool flipX;
    bool flipY;
    int xPos;

    int initPalette;
    int baseInstIndex;

    uint8_t *song;
    uint8_t wait;

    int tail;
    int count;
    Arrow arrows[NUM_ARROWS_IN_TRACK];

} Track;

void init_track(Track *track, bool transpose, bool flipX, bool flipY,
                int xPos, int initPalette, int baseInstIndex, uint8_t *song);
void update_track(Track *track, bool isBeatFrame);

#endif //_TRACK_H

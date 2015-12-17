#ifndef _TRACK_H
#define _TRACK_H

#define NUM_ARROWS_IN_TRACK 16
#define RECEPTOR_PALETTE 2
#define ACTIVE_PALETTE 3
#define MISSED_PALETTE 4

struct __Track;

typedef struct __Arrow {
    int instIndex;
    int paletteIndex;
    int yPos;
    bool enabled;
    int freezeFrames;
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

    int receptorOam;
    int hitCount;
    bool wasActive;

    int tail;
    int count;
    Arrow arrows[NUM_ARROWS_IN_TRACK];

} Track;

void init_track(Track *track, bool transpose, bool flipX, bool flipY,
                int xPos, int initPalette, int baseInstIndex, uint8_t *song,
                int receptorOam, bool receptorFlip);
void update_track(Track *track, bool isBeatFrame);

void control_track(Track *track, bool isActive);

#endif //_TRACK_H

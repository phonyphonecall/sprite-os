#ifndef _TRACK_H
#define _TRACK_H

#define FLOOR_DIV(n, d) ((n)/(d))
#define ROUND_DIV(n, d) (((n) + (d)/2)/(d))
#define OFF_DIV(n, d, o) (((n) + (d)-(o))/(d))
#define CEIL_DIV(n, d) (((n) + (d)-1)/(d))
#define abs(a) ((a) < 0 ? -(a) : (a))

#define SENSOR_Y_POS     10
#define BPM              140
#define PIXELS_PER_TICK  16
#define TICKS_ACTIVE     4

#define VS_PER_TICK      ROUND_DIV(3600, BPM*2)
#define PIXELS_PER_VSYNC CEIL_DIV(PIXELS_PER_TICK, VS_PER_TICK)
#define RESET_TTL        CEIL_DIV(480+64, PIXELS_PER_VSYNC)
#define ACT_TIME         (PIXELS_PER_VSYNC * VS_PER_TICK * TICKS_ACTIVE)
#define RESET_TTP        CEIL_DIV(480-SENSOR_Y_POS, PIXELS_PER_VSYNC)
#define PERFECT_TTL      (RESET_TTL - RESET_TTP)
#define MAX_ACT_TTL      (PERFECT_TTL + ACT_TIME/2)
#define MIN_ACT_TTL      (MAX_ACT_TTL - ACT_TIME)
#define TRACK_HIT_COUNT_RESET (VS_PER_TICK*2)
#define ARROW_INIT_Y     (SENSOR_Y_POS + RESET_TTP * PIXELS_PER_VSYNC)


#define NUM_ARROWS_IN_TRACK 16
#define RECEPTOR_PALETTE 3
#define ACTIVE_PALETTE 4
#define MISSED_PALETTE 5
#define ARROW_BASE_PALETTE 0x10
#define ARROW_BASE_OFFSET (16 - (RESET_TTP % 16))

struct __Track;

typedef struct __Arrow {
    int instIndex;
    int paletteOffset;
    int yPos;

    bool visible;
    int ttl;

    struct __Track *track;
} Arrow;

typedef struct __Track {
    bool transpose;
    bool flipX;
    bool flipY;
    int xPos;

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
                int xPos, int baseInstIndex, uint8_t *song,
                int receptorOam, bool receptorFlip);
void update_track(Track *track, bool isBeatFrame, int tickCount);

void control_track(Track *track, bool isActive);

#endif //_TRACK_H

#include "graphics.h"
#include "uart.h"
#include "track.h"

void init_track(Track *track, bool transpose, bool flipX, bool flipY,
        int xPos, int initPalette, int baseInstIndex) {
    track->transpose = transpose;
    track->flipX = flipX;
    track->flipY = flipY;
    track->xPos = xPos;
    track->initPalette = initPalette;
    track->baseInstIndex = baseInstIndex;
    track->tail = 0;
    track->count = 0;
}

void spawn_arrow(Track *track) {
    if (track->count == NUM_ARROWS_IN_TRACK) {
        sos_uart_printf("Error: Track has no more arrows!\n");
        return;
    }

    int index = (track->tail + track->count) % NUM_ARROWS_IN_TRACK;
    Arrow *arrow = &track->arrows[index];
    
    arrow->track = track;
    arrow->instIndex = track->baseInstIndex + index;
    arrow->yPos = 480;
    arrow->paletteIndex = track->initPalette;

    sos_inst_set(arrow->instIndex, OBJ_64x64, 0,
        track->transpose, true, arrow->paletteIndex,
        track->flipY, track->flipX, track->xPos, arrow->yPos);

    track->count++;
}

void delete_arrow(Arrow *arrow) {
    if (arrow->track->count == 0) {
        sos_uart_printf("Error: Tried to delete from empty track!\n");
        return;
    }
    if (arrow != &arrow->track->arrows[arrow->track->tail]) {
        arrow = &arrow->track->arrows[arrow->track->tail];
        sos_uart_printf("Error: Tried to delete arrows out of order!\n");
    }

    sos_inst_oam_set(arrow->instIndex, false, 0, false, false, 0, 0);

    arrow->track->count--;
    arrow->track->tail++;
    arrow->track->tail %= NUM_ARROWS_IN_TRACK;
}

void update_arrow(Arrow *arrow, int speed) {
    arrow->yPos -= speed;
    if (arrow->yPos <= -64) {
        delete_arrow(arrow);
        return;
    }

    Track *track = arrow->track;
    sos_inst_oam_set(arrow->instIndex, true, arrow->paletteIndex,
        track->flipY, track->flipX, track->xPos, arrow->yPos);
}

void update_track(Track *track, bool isBeatFrame) {
    if (isBeatFrame) {
        bool createNewArrow = true;
        if (createNewArrow) {
            spawn_arrow(track);
        }
    }

    int num = track->count;
    for (int c = 0, idx = track->tail; c < num;
        c++, idx = (idx+1) % NUM_ARROWS_IN_TRACK) {

        update_arrow(&track->arrows[idx], 1);
    }
}


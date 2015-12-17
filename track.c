#include "graphics.h"
#include "uart.h"
#include "track.h"

#define TRACK_HIT_COUNT_RESET 30
#define RESET_FREEZE_FRAMES 3
#define SENSOR_X_POS 10

void init_track(Track *track, bool transpose, bool flipX, bool flipY,
        int xPos, int initPalette, int baseInstIndex, uint8_t *song,
        int receptorOam, bool receptorFlip) {
    track->transpose = transpose;
    track->flipX = flipX;
    track->flipY = flipY;
    track->xPos = xPos;
    track->initPalette = initPalette;
    track->baseInstIndex = baseInstIndex;
    track->song = song;
    track->wait = *song;
    track->tail = 0;
    track->count = 0;
    track->receptorOam = receptorOam;
    track->hitCount = 0;
    track->wasActive = false;
    sos_oam_set(receptorOam, true, RECEPTOR_PALETTE,  receptorFlip, receptorFlip, xPos, SENSOR_X_POS);
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
    arrow->enabled = true;
    arrow->freezeFrames = 0;

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
    if (arrow->enabled) {
        arrow->yPos -= speed;
    } else if (arrow->freezeFrames > 0) {
        arrow->freezeFrames--;
    } else {
        delete_arrow(arrow);
        return;
    }
    if (arrow->yPos <= -64) {
        // TODO: TTL
        delete_arrow(arrow);
        return;
    }

    Track *track = arrow->track;
    sos_inst_oam_set(arrow->instIndex, true, arrow->paletteIndex,
        track->flipY, track->flipX, track->xPos, arrow->yPos);
}

void update_track(Track *track, bool isBeatFrame) {
    if (isBeatFrame) {
        if (track->wait == 0) {
            spawn_arrow(track);
            track->song++;
            track->wait = *track->song;
        }
        if (track->wait != 0xFF) {
            track->wait--;
        }
    }

    int num = track->count;
    for (int c = 0, idx = track->tail; c < num;
        c++, idx = (idx+1) % NUM_ARROWS_IN_TRACK) {

        update_arrow(&track->arrows[idx], 4);
    }
}

void hit_arrow(Arrow *arrow) {
    arrow->enabled = false;
    arrow->freezeFrames = RESET_FREEZE_FRAMES;
}

bool activate_track(Track *track) {
    for (int iter = track->tail, num = track->count; num > 0; num--, iter = (iter+1) % NUM_ARROWS_IN_TRACK) {
        Arrow *arrow = &track->arrows[iter];
        if (!arrow->enabled) continue;
        // TODO: timing, not pixels
        if (arrow->yPos - SENSOR_X_POS > 64)
            break; // checks if we've passed the target

        hit_arrow(arrow);
        track->hitCount = TRACK_HIT_COUNT_RESET;
        return true;
    }
    return false;
}

void control_track(Track *track, bool isActive) {
    if (!track->wasActive && isActive) {
        activate_track(track);
    }

    sos_oam_update(track->receptorOam,
        if (isActive) {
            if (track->hitCount > 0) {
                SOS_OAM_PALETTE=ACTIVE_PALETTE;
                track->hitCount--;
            } else {
                SOS_OAM_PALETTE=MISSED_PALETTE;
            }
        } else {
            SOS_OAM_PALETTE=RECEPTOR_PALETTE;
            track->hitCount = 0;
        }
    );

    track->wasActive = isActive;
}


#include "graphics.h"
#include "uart.h"
#include "track.h"


void init_track(Track *track, bool transpose, bool flipX, bool flipY,
        int xPos, int baseInstIndex, uint8_t *song,
        int receptorOam, bool receptorFlip) {
    track->transpose = transpose;
    track->flipX = flipX;
    track->flipY = flipY;
    track->xPos = xPos;
    track->baseInstIndex = baseInstIndex;
    track->song = song;
    track->wait = *song;
    track->tail = 0;
    track->count = 0;
    track->receptorOam = receptorOam;
    track->hitCount = 0;
    track->wasActive = false;
    sos_oam_set(receptorOam, true, RECEPTOR_PALETTE,  receptorFlip, receptorFlip, xPos, SENSOR_Y_POS);
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
    arrow->yPos = ARROW_INIT_Y;
    arrow->paletteOffset = 0; // TODO
    arrow->ttl = RESET_TTL;
    arrow->visible = true;

    sos_inst_set(arrow->instIndex, OBJ_64x64, 0,
        track->transpose, true, arrow->paletteOffset + ARROW_BASE_PALETTE,
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

    if (arrow->visible) {
        sos_uart_printf("Missed!\n");
    }

    sos_inst_oam_set(arrow->instIndex, false, 0, false, false, 0, 0);

    arrow->track->count--;
    arrow->track->tail++;
    arrow->track->tail %= NUM_ARROWS_IN_TRACK;
}

void update_arrow(Arrow *arrow, int tickCount) {
    if (arrow->ttl > MAX_ACT_TTL) { // arrow is not yet active
        // TODO: palette swaps
    } else if (arrow->ttl >= MIN_ACT_TTL) { // arrow is active
        // TODO: palette swaps
    } else if (arrow->ttl > 0) { // arrow is past active
        // TODO: palette swaps
    } else { // arrow is expired
        delete_arrow(arrow);
        return;
    }

    arrow->yPos -= PIXELS_PER_VSYNC;
    arrow->ttl--;

    Track *track = arrow->track;
    sos_inst_oam_set(arrow->instIndex, arrow->visible, ARROW_BASE_PALETTE + arrow->paletteOffset + tickCount,
        track->flipY, track->flipX, track->xPos, arrow->yPos);
}

void update_track(Track *track, bool isBeatFrame, int tickCount) {
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

        update_arrow(&track->arrows[idx], tickCount);
    }
}

void hit_arrow(Arrow *arrow) {
    int score = OFF_DIV(3*abs(arrow->ttl - PERFECT_TTL), ACT_TIME/2, 4);
    switch(score) {
        case 0:
            sos_uart_printf("PERFECT!\n");
            break;
        case 1:
            sos_uart_printf("GREAT!\n");
            break;
        case 2:
            sos_uart_printf("Good.\n");
            break;
        case 3:
            sos_uart_printf("Ok.\n");
            break;
        default:
            sos_uart_printf("Unknown?\n");
            break;
    }
    arrow->visible = false;
}

bool activate_track(Track *track) {
    for (int iter = track->tail, num = track->count; num > 0; num--, iter = (iter+1) % NUM_ARROWS_IN_TRACK) {
        Arrow *arrow = &track->arrows[iter];
        if (!arrow->visible) continue;
        if (arrow->ttl < MIN_ACT_TTL) continue;
        if (arrow->ttl > MAX_ACT_TTL) break;

        // We hit an arrow!
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


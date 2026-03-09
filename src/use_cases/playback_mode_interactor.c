/*
    Playback Mode Interactor Implementation
 */

#include "playback_mode_interactor.h"

void playback_mode_init(PlaybackModeState *s) {
    s->current_mode = PLAYBACK_MODE_FULLSCREEN;
    s->video_finished = 0;
}

void playback_mode_switch(PlaybackModeState *s, PlaybackMode mode) {
    s->current_mode = mode;
    s->video_finished = 0;
}

void playback_mode_on_video_finished(PlaybackModeState *s) {
    s->video_finished = 1;
    /* Transition to minimized mode when video finishes */
    if (s->current_mode == PLAYBACK_MODE_FULLSCREEN) {
        s->current_mode = PLAYBACK_MODE_MINIMIZED;
    }
}

PlaybackMode playback_mode_get_current(PlaybackModeState *s) {
    return s->current_mode;
}

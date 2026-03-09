/*
    Playback Mode Interactor
    Manages transitions between fullscreen and minimized playback modes.
    Lives in use_cases layer to enforce clean architecture.
 */

#ifndef PLAYBACK_MODE_INTERACTOR_H
#define PLAYBACK_MODE_INTERACTOR_H

#include "playback_mode.h"

typedef struct {
    PlaybackMode current_mode;
    int video_finished;
} PlaybackModeState;

void playback_mode_init(PlaybackModeState *s);
void playback_mode_switch(PlaybackModeState *s, PlaybackMode mode);
void playback_mode_on_video_finished(PlaybackModeState *s);
PlaybackMode playback_mode_get_current(PlaybackModeState *s);

#endif

/*
    Play / pause / quit orchestration.
 */

#ifndef PLAYBACK_INTERACTOR_H
#define PLAYBACK_INTERACTOR_H

#include "playback_state.h"

/* ---------- Input Boundary ---------- */
typedef enum {
    PB_ACTION_TOGGLE_PAUSE,
    PB_ACTION_QUIT
} PlaybackAction;

/* ---------- Output Boundary ---------- */
typedef struct {
    int paused;     /* new pause state after action */
    int quit;       /* 1 if quit was requested      */
} PlaybackResult;

/*
 * Execute a playback action.  Pure function.
 * Mutates the entity, returns result for the presenter.
 */
PlaybackResult playback_execute(PlaybackState *ps, PlaybackAction action);

#endif

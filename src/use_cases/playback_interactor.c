/*
    Play / pause / quit.
 */

#include "playback_interactor.h"

PlaybackResult playback_execute(PlaybackState *ps, PlaybackAction action) {
    PlaybackResult res;

    switch (action) {
        case PB_ACTION_TOGGLE_PAUSE:
            playback_toggle_pause(ps);
            break;
        case PB_ACTION_QUIT:
            playback_request_quit(ps);
            break;
    }

    res.paused = ps->paused;
    res.quit   = ps->quit;
    return res;
}

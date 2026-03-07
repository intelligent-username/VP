/*
    Playback control flags.
 */

#include "playback_state.h"

void playback_state_init(PlaybackState *ps) {
    ps->paused       = 0;
    ps->quit         = 0;
    ps->duration_sec = 0.0;
}

void playback_toggle_pause(PlaybackState *ps) {
    ps->paused = !ps->paused;
}

void playback_request_quit(PlaybackState *ps) {
    ps->quit = 1;
}

void playback_set_duration(PlaybackState *ps, double dur) {
    ps->duration_sec = dur;
}

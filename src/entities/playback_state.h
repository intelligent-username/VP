/*
    Playback control flags.
 */

#ifndef PLAYBACK_STATE_H
#define PLAYBACK_STATE_H

typedef struct {
    int    paused;
    int    quit;
    double duration_sec;   /* total media duration */
} PlaybackState;

void playback_state_init(PlaybackState *ps);
void playback_toggle_pause(PlaybackState *ps);
void playback_request_quit(PlaybackState *ps);
void playback_set_duration(PlaybackState *ps, double dur);

#endif /* PLAYBACK_STATE_H */

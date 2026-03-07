/*
    Pure A/V clock tracking.
 */

#include "media_clock.h"

void clock_reset(MediaClock *c) {
    c->pts     = 0.0;
    c->started = 0;
}

void clock_seed(MediaClock *c, double initial_pts) {
    c->pts     = initial_pts;
    c->started = 1;
}

void clock_advance(MediaClock *c, double seconds) {
    c->pts += seconds;
}

double clock_get(const MediaClock *c) {
    return c->pts;
}

int clock_is_started(const MediaClock *c) {
    return c->started;
}

/*
    A/V clock tracking.
 */

#ifndef MEDIA_CLOCK_H
#define MEDIA_CLOCK_H

typedef struct {
    double pts;        /* last known presentation timestamp (seconds) */
    int    started;    /* 1 once the first PTS has been seeded        */
} MediaClock;

/* Reset clock to zero / unstarted */
void  clock_reset(MediaClock *c);

/* Seed the clock with the first PTS value */
void  clock_seed(MediaClock *c, double initial_pts);

/* Advance the clock by a duration in seconds */
void  clock_advance(MediaClock *c, double seconds);

/* Read the current PTS */
double clock_get(const MediaClock *c);

/* Return 1 if the clock has been seeded */
int   clock_is_started(const MediaClock *c);

#endif /* MEDIA_CLOCK_H */

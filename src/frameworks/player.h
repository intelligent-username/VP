/*
    Video player lifecycle.
 */

#ifndef PLAYER_H
#define PLAYER_H

#include "home_presenter.h"
#include "app_context.h"

typedef struct {
    HomePresenter *hp;       /* borrowed: for view switching */
    AppContext    *ctx;       /* owned: created per playback  */
    guint          timer_id; /* GTK timeout for render ticks  */
} Player;

/* Start playing a video file.
 * Launches decode threads and a GTK render timer. */
void player_start(Player *p, HomePresenter *hp, const char *filepath);

/* Stop playback, clean up, return to home. */
void player_stop(Player *p);

#endif

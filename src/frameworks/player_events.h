/*
    Player event handlers.
    Manages keyboard, mouse, and playback mode transitions.
 */

#ifndef PLAYER_EVENTS_H
#define PLAYER_EVENTS_H

#include <gtk/gtk.h>
#include "player.h"
#include "home_presenter.h"

/* Keyboard events: spacebar for pause/play */
gboolean player_events_on_key_press(GtkWidget *w, GdkEventKey *ev, gpointer userdata);

/* Mouse click events: click video to pause/play */
gboolean player_events_on_video_click(GtkWidget *w, GdkEventButton *ev, gpointer userdata);

/* Playback mode transitions */
void player_events_on_minimize(void *userdata);
void player_events_on_fullscreen(void *userdata);
void player_events_on_close(void *userdata);

#endif

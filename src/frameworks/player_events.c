/*
    Player event handlers.
 */

#include "player_events.h"
#include <SDL2/SDL.h>

gboolean player_events_on_key_press(GtkWidget *w, GdkEventKey *ev, gpointer userdata) {
    (void)w;
    Player *p = (Player *)userdata;
    if (!p->ctx) return FALSE;

    if (ev->keyval == GDK_KEY_space) {
        p->ctx->playback.paused = !p->ctx->playback.paused;
        SDL_PauseAudio(p->ctx->playback.paused);
        return TRUE;
    }
    return FALSE;
}

gboolean player_events_on_video_click(GtkWidget *w, GdkEventButton *ev, gpointer userdata) {
    (void)w; (void)ev;
    Player *p = (Player *)userdata;
    if (!p->ctx) return FALSE;

    p->ctx->playback.paused = !p->ctx->playback.paused;
    SDL_PauseAudio(p->ctx->playback.paused);
    return TRUE;
}

void player_events_on_minimize(void *userdata) {
    Player *p = (Player *)userdata;
    if (!p->hp || !p->ctx) return;
    
    vp_set_minimized_mode(&p->hp->vp);
    home_presenter_show_home(p->hp);
}

void player_events_on_fullscreen(void *userdata) {
    Player *p = (Player *)userdata;
    if (!p->hp) return;

    vp_set_fullscreen_mode(&p->hp->vp);
    home_presenter_show_player(p->hp);
}

void player_events_on_close(void *userdata) {
    Player *p = (Player *)userdata;
    if (!p) return;
    player_stop(p);
}

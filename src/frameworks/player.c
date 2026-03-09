/*
    The video player.
    Uses GTK

    The render timer fires every 16ms (~60fps) and implements
    the master-audio-clock sync algorithm:

    T_audio = samples consumed by SDL callback / sample_rate
    T_video = PTS of current video frame

    if T_video > T_audio + 0.020: wait
    if T_video < T_audio - 0.050: drop
    otherwise display
 */

#include "player.h"
#include "player_renderer.h"
#include "player_events.h"
#include "context_factory.h"
#include "sdl_init.h"
#include "demuxer.h"
#include "audio_decoder.h"
#include "video_decoder.h"
#include <stdio.h>

#define RENDER_INTERVAL_MS 16



static gboolean on_render_tick(gpointer data) {
    Player *p = (Player *)data;

    if (p->ctx->playback.quit) {
        p->timer_id = 0;
        player_stop(p);
        return G_SOURCE_REMOVE;
    }

    if (player_renderer_is_playback_over(p)) {
        /* Playback ended: hide mini-player and return to home */
        vp_on_video_finished(&p->hp->vp);
        home_presenter_show_home(p->hp);
        /* Stop render loop; keep last rendered frame visible in presenter */
        p->timer_id = 0;
        if (p->ctx) {
            context_teardown(p->ctx);
            p->ctx = NULL;
        }
        if (p->hp) {
            home_presenter_clear_selection(p->hp);
        }
        return G_SOURCE_REMOVE;
    }

    if (!p->ctx->playback.paused)
        player_renderer_try_display_frames(p);

    return G_SOURCE_CONTINUE;
}

/* ---- keyboard handler ---- */

/* ---- click to pause ---- */

/* ---- public API ---- */

static void on_player_minimize(void *userdata) {
    player_events_on_minimize(userdata);
}

static void on_player_fullscreen(void *userdata) {
    player_events_on_fullscreen(userdata);
}

static void on_player_close(void *userdata) {
    player_events_on_close(userdata);
}

void player_start(Player *p, HomePresenter *hp, const char *filepath) {
    p->hp  = hp;
    p->ctx = context_create();
    if (!p->ctx) return;

    if (context_init_ipc(p->ctx) < 0 ||
        context_open_media(p->ctx, filepath) < 0 ||
        sdl_bootstrap() < 0 ||
        video_decoder_init(p->ctx) < 0) {
        context_teardown(p->ctx);
        p->ctx = NULL;
        return;
    }

    audio_decoder_init(p->ctx);
    demuxer_start(p->ctx);
    video_decoder_start(p->ctx);

    vp_hide_miniplayer(&hp->vp);

    /* Start in fullscreen playback mode */
    vp_set_fullscreen_mode(&hp->vp);

    /* wire input events */
    gtk_widget_set_can_focus(hp->window, TRUE);
    g_signal_connect(hp->window, "key-press-event",
                     G_CALLBACK(player_events_on_key_press), p);
    
    vp_set_expand_callback(&hp->vp, on_player_fullscreen, p);
    vp_set_close_callback(&hp->vp, on_player_close, p);
    vp_connect_input_with_callback(&hp->vp,
                                   hp->window,
                                   on_player_minimize,
                                   p,
                                   on_player_fullscreen,
                                   p);

    /* make drawing area clickable */
    gtk_widget_add_events(hp->vp.drawing_area,
                          GDK_BUTTON_PRESS_MASK);
    g_signal_connect(hp->vp.drawing_area, "button-press-event",
                     G_CALLBACK(player_events_on_video_click), p);

    /* start render timer */
    p->timer_id = g_timeout_add(RENDER_INTERVAL_MS, on_render_tick, p);
}

void player_stop(Player *p) {
    if (p->timer_id) {
        g_source_remove(p->timer_id);
        p->timer_id = 0;
    }
    if (p->ctx) {
        context_teardown(p->ctx);
        p->ctx = NULL;
    }
    if (p->hp) {
        vp_hide_miniplayer(&p->hp->vp);
        vp_set_progress(&p->hp->vp, 0.0);
        home_presenter_show_home(p->hp);
        home_presenter_clear_selection(p->hp);
    }
}

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
#include "context_factory.h"
#include "sdl_init.h"
#include "demuxer.h"
#include "audio_decoder.h"
#include "video_decoder.h"
#include "sync_interactor.h"
#include <stdio.h>

#define RENDER_INTERVAL_MS 16


#define AUDIO_LATENCY_SEC 0.150

static double get_audio_time(Player *p) {
    /* Subtract hardware buffering latency to find actual speaker time */
    double t = audio_gateway_get_position(&p->ctx->audio_gw);
    return t > AUDIO_LATENCY_SEC ? t - AUDIO_LATENCY_SEC : 0.0;
}

static int has_audio(Player *p) {
    return p->ctx->audio_stream_idx >= 0;
}

static double get_frame_rate(Player *p) {
    return av_q2d(p->ctx->video_stream->r_frame_rate);
}

/* ---- render tick ---- */

static void display_frame(Player *p, RGBFrame *f) {
    vp_set_frame(&p->hp->vp, f);
    clock_seed(&p->ctx->video_clock, f->pts);

    double dur = p->ctx->playback.duration_sec;
    if (dur > 0.0)
        vp_set_progress(&p->hp->vp, f->pts / dur);
}

static void try_display_frames(Player *p) {
    double audio_time = has_audio(p) ? get_audio_time(p) : -1.0;

    while (1) {
        RGBFrame *f_ptr = fq_peek(&p->ctx->frame_queue);
        if (!f_ptr) break;

        if (has_audio(p)) {
            SyncInputData sin;
            sin.video_pts  = f_ptr->pts;
            sin.audio_pts  = audio_time;
            sin.has_audio  = 1;
            sin.frame_rate = get_frame_rate(p);

            SyncOutputData sout = sync_evaluate(&sin);

            if (sout.action == SYNC_DELAY && sout.delay_ms > 0) {
                break;
            }

            RGBFrame f;
            fq_dequeue(&p->ctx->frame_queue, &f);

            if (sout.action == SYNC_DROP) {
                free(f.data);      /* frame too late, discard */
                continue;          /* check the next frame immediately */
            }

            display_frame(p, &f);
            free(f.data);
            break;  /* one frame per tick is enough */
        } else {
            /* No audio stream: Just pace roughly at GTK timer speed */
            RGBFrame f;
            fq_dequeue(&p->ctx->frame_queue, &f);
            display_frame(p, &f);
            free(f.data);
            break;
        }
    }
}


static int is_playback_over(Player *p) {
    return p->ctx->frame_queue.finished
        && fq_is_empty(&p->ctx->frame_queue);
}

static gboolean on_render_tick(gpointer data) {
    Player *p = (Player *)data;

    if (p->ctx->playback.quit || is_playback_over(p)) {
        p->timer_id = 0; /* Prevent player_stop from calling g_source_remove */
        player_stop(p);
        return G_SOURCE_REMOVE;
    }

    if (!p->ctx->playback.paused)
        try_display_frames(p);

    return G_SOURCE_CONTINUE;
}

/* ---- keyboard handler ---- */

static gboolean on_key_press(GtkWidget *w, GdkEventKey *ev,
                             gpointer data) {
    (void)w;
    Player *p = (Player *)data;
    if (!p->ctx) return FALSE;

    if (ev->keyval == GDK_KEY_space) {
        p->ctx->playback.paused = !p->ctx->playback.paused;
        SDL_PauseAudio(p->ctx->playback.paused);
        return TRUE;
    }
    if (ev->keyval == GDK_KEY_Escape) {
        player_stop(p);
        return TRUE;
    }
    return FALSE;
}

/* ---- click to pause ---- */

static gboolean on_video_click(GtkWidget *w, GdkEventButton *ev,
                               gpointer data) {
    (void)w; (void)ev;
    Player *p = (Player *)data;
    if (!p->ctx) return FALSE;

    p->ctx->playback.paused = !p->ctx->playback.paused;
    SDL_PauseAudio(p->ctx->playback.paused);
    return TRUE;
}

/* ---- public API ---- */

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

    /* wire input events */
    gtk_widget_set_can_focus(hp->window, TRUE);
    g_signal_connect(hp->window, "key-press-event",
                     G_CALLBACK(on_key_press), p);

    /* make drawing area clickable */
    gtk_widget_add_events(hp->vp.drawing_area,
                          GDK_BUTTON_PRESS_MASK);
    g_signal_connect(hp->vp.drawing_area, "button-press-event",
                     G_CALLBACK(on_video_click), p);

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
        vp_set_progress(&p->hp->vp, 0.0);
        home_presenter_show_home(p->hp);
        home_presenter_clear_selection(p->hp);
    }
}

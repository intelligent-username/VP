/*
    Player frame rendering and synchronization.
 */

#include "player_renderer.h"
#include "sync_interactor.h"
#include "audio_gateway.h"
#include <math.h>

#define AUDIO_LATENCY_SEC 0.150

static double get_audio_time(Player *p) {
    double t = audio_gateway_get_position(&p->ctx->audio_gw);
    return t > AUDIO_LATENCY_SEC ? t - AUDIO_LATENCY_SEC : 0.0;
}

static int has_audio(Player *p) {
    return p->ctx->audio_stream_idx >= 0;
}

static double get_frame_rate(Player *p) {
    return av_q2d(p->ctx->video_stream->r_frame_rate);
}

void player_renderer_display_frame(Player *p, RGBFrame *f) {
    vp_set_frame(&p->hp->vp, f);
    clock_seed(&p->ctx->video_clock, f->pts);

    double dur = p->ctx->playback.duration_sec;
    if (dur > 0.0)
        vp_set_progress(&p->hp->vp, f->pts / dur);
}

void player_renderer_try_display_frames(Player *p) {
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
                free(f.data);
                continue;
            }

            player_renderer_display_frame(p, &f);
            free(f.data);
            break;
        } else {
            RGBFrame f;
            fq_dequeue(&p->ctx->frame_queue, &f);
            player_renderer_display_frame(p, &f);
            free(f.data);
            break;
        }
    }
}

int player_renderer_is_playback_over(Player *p) {
    return p->ctx->frame_queue.finished && fq_is_empty(&p->ctx->frame_queue);
}

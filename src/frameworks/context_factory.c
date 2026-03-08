/*
    AppContext lifecycle.
 */

#include "context_factory.h"
#include "socket_ipc.h"
#include "demuxer.h"
#include "audio_decoder.h"
#include "video_decoder.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

AppContext *context_create(void) {
    AppContext *ctx = calloc(1, sizeof(AppContext));
    if (!ctx) { fprintf(stderr, "OOM\n"); return NULL; }
    playback_state_init(&ctx->playback);
    clock_reset(&ctx->audio_clock);
    clock_reset(&ctx->video_clock);
    fq_init(&ctx->frame_queue);
    ctx->video_sk_send = -1;
    ctx->video_sk_recv = -1;
    ctx->audio_sk_send = -1;
    ctx->audio_sk_recv = -1;
    return ctx;
}

int context_init_ipc(AppContext *ctx) {
    int vsocks[2], asocks[2];
    if (ipc_init_sockets(vsocks) < 0) return -1;
    if (ipc_init_sockets(asocks) < 0) return -1;
    ipc_increase_buffer(vsocks[0], 8 * 1024 * 1024);
    ipc_increase_buffer(asocks[0], 2 * 1024 * 1024);
    ctx->video_sk_send = vsocks[0];
    ctx->video_sk_recv = vsocks[1];
    ctx->audio_sk_send = asocks[0];
    ctx->audio_sk_recv = asocks[1];
    return 0;
}

int context_open_media(AppContext *ctx, const char *path) {
    if (avformat_open_input(&ctx->format_ctx, path, NULL, NULL) < 0) {
        fprintf(stderr, "Cannot open '%s'\n", path);
        return -1;
    }
    if (avformat_find_stream_info(ctx->format_ctx, NULL) < 0) {
        fprintf(stderr, "Cannot find stream info\n");
        return -1;
    }
    ctx->video_stream_idx = av_find_best_stream(
        ctx->format_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    ctx->audio_stream_idx = av_find_best_stream(
        ctx->format_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);

    if (ctx->format_ctx->duration != AV_NOPTS_VALUE)
        playback_set_duration(&ctx->playback,
            (double)ctx->format_ctx->duration / AV_TIME_BASE);
    return 0;
}

void context_close_sockets(AppContext *ctx) {
    if (ctx->video_sk_send >= 0) { close(ctx->video_sk_send); ctx->video_sk_send = -1; }
    if (ctx->video_sk_recv >= 0) { close(ctx->video_sk_recv); ctx->video_sk_recv = -1; }
    if (ctx->audio_sk_send >= 0) { close(ctx->audio_sk_send); ctx->audio_sk_send = -1; }
    if (ctx->audio_sk_recv >= 0) { close(ctx->audio_sk_recv); ctx->audio_sk_recv = -1; }
}

void context_teardown(AppContext *ctx) {
    playback_request_quit(&ctx->playback);
    fq_signal_finished(&ctx->frame_queue);
    context_close_sockets(ctx);
    demuxer_stop(ctx);
    video_decoder_cleanup(ctx);
    audio_decoder_cleanup(ctx);
    fq_destroy(&ctx->frame_queue);
    if (ctx->format_ctx) avformat_close_input(&ctx->format_ctx);
    free(ctx);
}

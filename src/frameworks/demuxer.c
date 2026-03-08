/*
    FFmpeg demuxer thread.
    Reads packets from the container and dispatches via IPC sockets.
    On EOF, closes send sockets so readers detect end-of-stream.
 */

#include "demuxer.h"
#include "socket_ipc.h"
#include <SDL2/SDL.h>
#include <unistd.h>

static int is_video_packet(AppContext *ctx, AVPacket *pkt) {
    return pkt->stream_index == ctx->video_stream_idx;
}

static int is_audio_packet(AppContext *ctx, AVPacket *pkt) {
    return pkt->stream_index == ctx->audio_stream_idx;
}

static int dispatch_packet(AppContext *ctx, AVPacket *pkt) {
    if (is_video_packet(ctx, pkt))
        return ipc_send_packet(ctx->video_sk_send, pkt);
    if (is_audio_packet(ctx, pkt))
        return ipc_send_packet(ctx->audio_sk_send, pkt);
    return 0;
}

static void close_send_sockets(AppContext *ctx) {
    if (ctx->video_sk_send >= 0) close(ctx->video_sk_send);
    if (ctx->audio_sk_send >= 0) close(ctx->audio_sk_send);
    ctx->video_sk_send = -1;
    ctx->audio_sk_send = -1;
}

static int demux_thread_fn(void *arg) {
    AppContext *ctx = (AppContext *)arg;
    AVPacket *pkt = av_packet_alloc();

    while (!ctx->playback.quit) {
        if (ctx->playback.paused) { SDL_Delay(20); continue; }

        if (av_read_frame(ctx->format_ctx, pkt) >= 0) {
            if (dispatch_packet(ctx, pkt) < 0)
                playback_request_quit(&ctx->playback);
            av_packet_unref(pkt);
        } else {
            /* EOF: close send sockets so receivers get errors */
            close_send_sockets(ctx);
            break;
        }
    }
    av_packet_free(&pkt);
    return 0;
}

int demuxer_start(AppContext *ctx) {
    ctx->demux_tid = SDL_CreateThread(demux_thread_fn, "DemuxTid", ctx);
    return ctx->demux_tid ? 0 : -1;
}

void demuxer_stop(AppContext *ctx) {
    if (ctx->demux_tid) {
        SDL_WaitThread(ctx->demux_tid, NULL);
        ctx->demux_tid = NULL;
    }
}

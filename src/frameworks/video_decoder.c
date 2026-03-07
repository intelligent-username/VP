/*
    Threaded video decoding.
    Receives packets via IPC, decodes, converts to BGRA,
    and enqueues to the bounded FrameQueue.
 */

#include "video_decoder.h"
#include "socket_ipc.h"
#include <stdio.h>

/* ---- codec init ---- */

int video_decoder_init(AppContext *ctx) {
    if (ctx->video_stream_idx < 0) {
        fprintf(stderr, "No video stream\n");
        return -1;
    }
    ctx->video_stream = ctx->format_ctx->streams[ctx->video_stream_idx];
    const AVCodec *codec = avcodec_find_decoder(
        ctx->video_stream->codecpar->codec_id);
    if (!codec) { fprintf(stderr, "Video codec not found\n"); return -1; }

    ctx->video_ctx = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(ctx->video_ctx,
        ctx->video_stream->codecpar);
    ctx->video_ctx->thread_count = 0;

    if (avcodec_open2(ctx->video_ctx, codec, NULL) < 0) {
        fprintf(stderr, "Cannot open video codec\n");
        return -1;
    }
    return 0;
}

/* ---- conversion helper ---- */

static struct SwsContext *create_sws(AVFrame *frame) {
    return sws_getContext(
        frame->width, frame->height, frame->format,
        frame->width, frame->height, AV_PIX_FMT_BGRA,
        SWS_BILINEAR, NULL, NULL, NULL);
}

static int convert_to_bgra(struct SwsContext *sws, AVFrame *frame,
                           RGBFrame *out) {
    int w = frame->width, h = frame->height;
    out->stride = (w * 4 + 31) & ~31; /* 32-byte align for AVX */
    out->width  = w;
    out->height = h;
    /* allocate with 64-byte padding to prevent sws_scale overrun */
    out->data   = malloc((out->stride * h) + 64);
    if (!out->data) return -1;

    uint8_t *dst[1]  = { out->data };
    int linesize[1]  = { out->stride };
    sws_scale(sws, (const uint8_t *const *)frame->data,
              frame->linesize, 0, h, dst, linesize);
    return 0;
}

static double frame_pts(AppContext *ctx, AVFrame *frame) {
    return frame->best_effort_timestamp
         * av_q2d(ctx->video_stream->time_base);
}

/* ---- decode thread ---- */

static int video_decode_thread_fn(void *arg) {
    AppContext *ctx = (AppContext *)arg;
    AVPacket *pkt   = av_packet_alloc();
    AVFrame  *frame = av_frame_alloc();
    struct SwsContext *sws = NULL;

    while (!ctx->playback.quit) {
        if (ctx->playback.paused) { SDL_Delay(20); continue; }

        int ret = ipc_recv_packet_timeout(ctx->video_sk_recv, pkt, 50);
        if (ret < 0) break;           /* EOF / error */
        if (ret == 0) continue;        /* timeout     */

        avcodec_send_packet(ctx->video_ctx, pkt);
        while (avcodec_receive_frame(ctx->video_ctx, frame) == 0) {
            if (!sws) sws = create_sws(frame);

            RGBFrame rf;
            rf.pts = frame_pts(ctx, frame);
            if (convert_to_bgra(sws, frame, &rf) == 0)
                fq_enqueue(&ctx->frame_queue, &rf);
        }
        av_packet_unref(pkt);
    }

    fq_signal_finished(&ctx->frame_queue);
    if (sws) sws_freeContext(sws);
    av_frame_free(&frame);
    av_packet_free(&pkt);
    return 0;
}

int video_decoder_start(AppContext *ctx) {
    ctx->video_tid = SDL_CreateThread(
        video_decode_thread_fn, "VideoDecode", ctx);
    return ctx->video_tid ? 0 : -1;
}

void video_decoder_cleanup(AppContext *ctx) {
    if (ctx->video_tid) {
        SDL_WaitThread(ctx->video_tid, NULL);
        ctx->video_tid = NULL;
    }
    if (ctx->video_ctx) avcodec_free_context(&ctx->video_ctx);
}

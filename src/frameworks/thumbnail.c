/*
    FFmpeg-based thumbnail extraction.
    ALL THUMBNAILS SHOULD BE 16:9 (cropped)
 */

#include "thumbnail.h"
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <stdlib.h>
#include <string.h>

/* ---- open / decode helpers ---- */

static int open_video(const char *path, AVFormatContext **fc,
                      int *stream_idx) {
    if (avformat_open_input(fc, path, NULL, NULL) < 0) return -1;
    if (avformat_find_stream_info(*fc, NULL) < 0)      return -1;
    *stream_idx = av_find_best_stream(*fc, AVMEDIA_TYPE_VIDEO,
                                       -1, -1, NULL, 0);
    return (*stream_idx >= 0) ? 0 : -1;
}

static AVCodecContext *open_decoder(AVFormatContext *fc, int idx) {
    const AVCodec *codec = avcodec_find_decoder(
        fc->streams[idx]->codecpar->codec_id);
    if (!codec) return NULL;
    AVCodecContext *ctx = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(ctx, fc->streams[idx]->codecpar);
    if (avcodec_open2(ctx, codec, NULL) < 0) {
        avcodec_free_context(&ctx);
        return NULL;
    }
    return ctx;
}

static void seek_to_quarter(AVFormatContext *fc) {
    if (fc->duration > 0) {
        int64_t target = fc->duration / 4;
        av_seek_frame(fc, -1, target, AVSEEK_FLAG_BACKWARD);
    }
}

static int decode_one_frame(AVFormatContext *fc, AVCodecContext *ctx,
                            int stream_idx, AVFrame *frame) {
    AVPacket *pkt = av_packet_alloc();
    int got_frame = 0;

    while (!got_frame && av_read_frame(fc, pkt) >= 0) {
        if (pkt->stream_index == stream_idx) {
            if (avcodec_send_packet(ctx, pkt) == 0) {
                if (avcodec_receive_frame(ctx, frame) == 0)
                    got_frame = 1;
            }
        }
        av_packet_unref(pkt);
    }
    av_packet_free(&pkt);
    return got_frame ? 0 : -1;
}

/* ---- 16:9 cover-crop helpers ---- */

static void compute_cover_scale(int src_w, int src_h,
                                int dst_w, int dst_h,
                                int *inter_w, int *inter_h) {
    double sx = (double)dst_w / src_w;
    double sy = (double)dst_h / src_h;
    double s  = (sx > sy) ? sx : sy;
    *inter_w = (int)(src_w * s);
    *inter_h = (int)(src_h * s);
    /* ensure at least dst dimensions */
    if (*inter_w < dst_w) *inter_w = dst_w;
    if (*inter_h < dst_h) *inter_h = dst_h;
}

static uint8_t *scale_frame_to_rgb(AVFrame *src, int dst_w, int dst_h, int *out_stride) {
    struct SwsContext *sws = sws_getContext(
        src->width, src->height, src->format,
        dst_w, dst_h, AV_PIX_FMT_RGB24,
        SWS_BILINEAR, NULL, NULL, NULL);
    if (!sws) return NULL;

    *out_stride = (dst_w * 3 + 31) & ~31;
    uint8_t *buf = malloc((*out_stride * dst_h) + 64);
    uint8_t *dst_data[1]  = { buf };
    int      dst_line[1]  = { *out_stride };
    sws_scale(sws, (const uint8_t *const *)src->data,
              src->linesize, 0, src->height,
              dst_data, dst_line);
    sws_freeContext(sws);
    return buf;
}

static void crop_center_rgb(const uint8_t *src, int src_w, int src_h, int src_stride,
                            uint8_t *dst, int dst_w, int dst_h) {
    int off_x = (src_w - dst_w) / 2;
    int off_y = (src_h - dst_h) / 2;
    for (int y = 0; y < dst_h; y++) {
        const uint8_t *row = src + (y + off_y) * src_stride + off_x * 3;
        memcpy(dst + y * dst_w * 3, row, dst_w * 3);
    }
}

/* ---- public API ---- */

int thumbnail_extract(const char *path, ThumbnailData *out,
                      int target_w) {
    AVFormatContext *fc = NULL;
    int idx;
    if (open_video(path, &fc, &idx) < 0) return -1;

    AVCodecContext *ctx = open_decoder(fc, idx);
    if (!ctx) { avformat_close_input(&fc); return -1; }

    seek_to_quarter(fc);

    AVFrame *frame = av_frame_alloc();
    int ret = decode_one_frame(fc, ctx, idx, frame);

    if (ret == 0) {
        int target_h = target_w * 9 / 16;
        int inter_w, inter_h;
        compute_cover_scale(frame->width, frame->height,
                            target_w, target_h,
                            &inter_w, &inter_h);

        int inter_stride = 0;
        /* Scale to intermediate "cover" size */
        uint8_t *inter = scale_frame_to_rgb(frame, inter_w, inter_h, &inter_stride);
        if (!inter) { ret = -1; goto cleanup; }

        /* Crop center to exact 16:9 */
        out->data   = malloc(target_w * target_h * 3);
        out->width  = target_w;
        out->height = target_h;
        crop_center_rgb(inter, inter_w, inter_h, inter_stride,
                        out->data, target_w, target_h);
        free(inter);
    }

cleanup:
    av_frame_free(&frame);
    avcodec_free_context(&ctx);
    avformat_close_input(&fc);
    return ret;
}

void thumbnail_free(ThumbnailData *td) {
    free(td->data);
    td->data = NULL;
}

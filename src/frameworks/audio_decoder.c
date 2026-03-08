/*
    Audio decode thread.
 
    Receives compressed packets via IPC, decodes + resamples to S16,
    writes to FIFO.  The audio_gateway (SDL callback) drains the FIFO
    and advances the audio clock, so the clock tracks playback.
 */

#include "audio_decoder.h"
#include "audio_gateway.h"
#include "socket_ipc.h"
#include <stdio.h>


static int open_audio_codec(AppContext *ctx) {
    AVStream *st = ctx->format_ctx->streams[ctx->audio_stream_idx];
    const AVCodec *codec = avcodec_find_decoder(st->codecpar->codec_id);
    if (!codec) { fprintf(stderr, "Audio codec not found\n"); return -1; }

    ctx->audio_ctx = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(ctx->audio_ctx, st->codecpar);
    if (avcodec_open2(ctx->audio_ctx, codec, NULL) < 0) {
        fprintf(stderr, "Cannot open audio codec\n"); return -1;
    }
    ctx->audio_stream = st;
    return 0;
}

static int init_resampler(AppContext *ctx) {
    AVCodecContext *ac = ctx->audio_ctx;
    av_channel_layout_copy(&ctx->audio_ch_layout, &ac->ch_layout);
    ctx->audio_sample_rate = ac->sample_rate;

    swr_alloc_set_opts2(&ctx->swr,
        &ac->ch_layout, AV_SAMPLE_FMT_S16, ac->sample_rate,
        &ac->ch_layout, ac->sample_fmt,    ac->sample_rate,
        0, NULL);
    return swr_init(ctx->swr);
}

static int init_fifo_and_mutex(AppContext *ctx) {
    ctx->audio_fifo = av_audio_fifo_alloc(
        AV_SAMPLE_FMT_S16, ctx->audio_ch_layout.nb_channels, 8192);
    ctx->audio_mutex = SDL_CreateMutex();
    return (ctx->audio_fifo && ctx->audio_mutex) ? 0 : -1;
}

static int open_sdl_audio(AppContext *ctx) {
    SDL_AudioSpec wanted = {0};
    wanted.freq     = ctx->audio_sample_rate;
    wanted.format   = AUDIO_S16SYS;
    wanted.channels = (Uint8)ctx->audio_ch_layout.nb_channels;
    wanted.samples  = 1024;
    wanted.callback = audio_gateway_callback;
    wanted.userdata = &ctx->audio_gw;

    if (SDL_OpenAudio(&wanted, NULL) < 0) {
        fprintf(stderr, "SDL_OpenAudio: %s\n", SDL_GetError());
        return -1;
    }
    return 0;
}

static void setup_gateway(AppContext *ctx) {
    ctx->audio_gw.fifo        = ctx->audio_fifo;
    ctx->audio_gw.mutex       = ctx->audio_mutex;
    ctx->audio_gw.clock       = &ctx->audio_clock;
    ctx->audio_gw.sample_rate = ctx->audio_sample_rate;
    ctx->audio_gw.channels    = ctx->audio_ch_layout.nb_channels;
}

static void wait_if_fifo_full(AppContext *ctx) {
    while (!ctx->playback.quit &&
           av_audio_fifo_size(ctx->audio_fifo) > 96000) {
        SDL_Delay(10);
    }
}

static void seed_clock_if_needed(AppContext *ctx, AVFrame *frame) {
    if (clock_is_started(&ctx->audio_clock)) return;
    if (frame->best_effort_timestamp == AV_NOPTS_VALUE) return;

    double pts = frame->best_effort_timestamp
               * av_q2d(ctx->audio_stream->time_base);
    SDL_LockMutex(ctx->audio_mutex);
    clock_seed(&ctx->audio_clock, pts);
    SDL_UnlockMutex(ctx->audio_mutex);
}

static void resample_and_enqueue(AppContext *ctx, AVFrame *frame) {
    if (frame->nb_samples <= 0) return;

    uint8_t *out = NULL;
    int out_samples = (int)av_rescale_rnd(
        swr_get_delay(ctx->swr, ctx->audio_ctx->sample_rate)
            + frame->nb_samples,
        ctx->audio_ctx->sample_rate, ctx->audio_ctx->sample_rate,
        AV_ROUND_UP);

    if (out_samples <= 0) return;

    if (av_samples_alloc(&out, NULL, ctx->audio_ch_layout.nb_channels,
                         out_samples, AV_SAMPLE_FMT_S16, 0) < 0) {
        return; /* Allocation failed (corrupt huge packet?), drop cleanly */
    }

    int converted = swr_convert(ctx->swr, &out, out_samples,
        (const uint8_t **)frame->data, frame->nb_samples);

    if (converted > 0) {
        /* "Normalize" deep-fried audio using a simple peak limiter / AGC */
        int16_t *samples = (int16_t *)out;
        int total_samples = converted * ctx->audio_ch_layout.nb_channels;
        int32_t peak = 0;
        
        for (int i = 0; i < total_samples; i++) {
            int32_t val = samples[i];
            if (val < 0) val = -val;
            if (val > peak) peak = val;
        }

        /* If volume exceeds dynamic headroom (e.g. 24000), crush it down smoothly */
        if (peak > 24000) {
            double scale = 24000.0 / (double)peak;
            for (int i = 0; i < total_samples; i++) {
                samples[i] = (int16_t)(samples[i] * scale);
            }
        }

        wait_if_fifo_full(ctx);
        SDL_LockMutex(ctx->audio_mutex);
        av_audio_fifo_write(ctx->audio_fifo, (void **)&out, converted);
        SDL_UnlockMutex(ctx->audio_mutex);
    }
    if (out) av_freep(&out);
}

/* ---- decode thread ---- */

static int audio_decode_thread_fn(void *arg) {
    AppContext *ctx = (AppContext *)arg;
    AVPacket *pkt   = av_packet_alloc();
    AVFrame  *frame = av_frame_alloc();

    while (!ctx->playback.quit) {
        if (ctx->playback.paused) { SDL_Delay(20); continue; }

        int ret = ipc_recv_packet_timeout(ctx->audio_sk_recv, pkt, 50);
        if (ret < 0) break;    /* EOF / error */
        if (ret == 0) continue; /* timeout */

        if (avcodec_send_packet(ctx->audio_ctx, pkt) == 0) {
            while (avcodec_receive_frame(ctx->audio_ctx, frame) == 0) {
                seed_clock_if_needed(ctx, frame);
                resample_and_enqueue(ctx, frame);
            }
        }
        av_packet_unref(pkt);
    }
    av_frame_free(&frame);
    av_packet_free(&pkt);
    return 0;
}

/* ---- public API ---- */

int audio_decoder_init(AppContext *ctx) {
    if (ctx->audio_stream_idx < 0) return 0;

    if (open_audio_codec(ctx) < 0)     return -1;
    if (init_resampler(ctx)  < 0)      return -1;
    if (init_fifo_and_mutex(ctx) < 0)  return -1;

    setup_gateway(ctx);

    if (open_sdl_audio(ctx) < 0) return -1;

    ctx->audio_tid = SDL_CreateThread(
        audio_decode_thread_fn, "AudioDecode", ctx);
    SDL_PauseAudio(0);
    return 0;
}

void audio_decoder_cleanup(AppContext *ctx) {
    if (ctx->audio_tid) {
        SDL_WaitThread(ctx->audio_tid, NULL);
        ctx->audio_tid = NULL;
    }
    SDL_CloseAudio();
    if (ctx->swr)         swr_free(&ctx->swr);
    if (ctx->audio_fifo)  av_audio_fifo_free(ctx->audio_fifo);
    if (ctx->audio_mutex) SDL_DestroyMutex(ctx->audio_mutex);
    if (ctx->audio_ctx)   avcodec_free_context(&ctx->audio_ctx);
}

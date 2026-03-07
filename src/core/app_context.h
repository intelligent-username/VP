/*
    Core: Central application state.

    Video rendering is now done via GTK/Cairo (no SDL video).
    SDL is used ONLY for audio playback.

    The FrameQueue bridges the video decode thread and the GTK
    render timer, keeping at most FRAME_QUEUE_SIZE frames in RAM.
 */

#ifndef APP_CONTEXT_H
#define APP_CONTEXT_H

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/audio_fifo.h>
#include <libavutil/samplefmt.h>
#include <libavutil/channel_layout.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
#include <SDL2/SDL.h>

#include "media_clock.h"
#include "playback_state.h"
#include "audio_gateway.h"
#include "frame_queue.h"

typedef struct AppContext {
    /* ---- Entities ---- */
    PlaybackState   playback;
    MediaClock      audio_clock;
    MediaClock      video_clock;
    FrameQueue      frame_queue;

    /* ---- Format / streams ---- */
    AVFormatContext *format_ctx;
    int              video_stream_idx;
    int              audio_stream_idx;
    AVStream        *video_stream;
    AVStream        *audio_stream;

    /* ---- Video decoder ---- */
    AVCodecContext  *video_ctx;

    /* ---- Audio decoder ---- */
    AVCodecContext  *audio_ctx;
    SwrContext      *swr;
    AVChannelLayout  audio_ch_layout;
    int              audio_sample_rate;

    /* ---- Audio FIFO + guard ---- */
    AVAudioFifo     *audio_fifo;
    SDL_mutex       *audio_mutex;

    /* ---- Interface adapters ---- */
    AudioGateway     audio_gw;

    /* ---- IPC sockets ---- */
    int              video_sk_send;
    int              video_sk_recv;
    int              audio_sk_send;
    int              audio_sk_recv;

    /* ---- Threads ---- */
    SDL_Thread      *demux_tid;
    SDL_Thread      *audio_tid;
    SDL_Thread      *video_tid;

} AppContext;

#endif

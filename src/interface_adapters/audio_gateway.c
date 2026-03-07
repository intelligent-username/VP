/*
    Audio Gateway:
    SDL audio callback -> FIFO drain -> clock advance.
 */

#include "audio_gateway.h"

static int compute_samples_needed(int len, int channels) {
    int bps = av_get_bytes_per_sample(AV_SAMPLE_FMT_S16) * channels;
    return len / bps;
}

static int drain_fifo(AudioGateway *gw, Uint8 *stream, int samples) {
    int avail = av_audio_fifo_size(gw->fifo);
    int to_read = avail < samples ? avail : samples;
    if (to_read > 0)
        av_audio_fifo_read(gw->fifo, (void **)&stream, to_read);
    return to_read;
}

void audio_gateway_callback(void *userdata, Uint8 *stream, int len) {
    AudioGateway *gw = (AudioGateway *)userdata;
    SDL_memset(stream, 0, len);

    if (!gw->fifo || !gw->mutex) return;

    int needed = compute_samples_needed(len, gw->channels);

    SDL_LockMutex(gw->mutex);
    int read = drain_fifo(gw, stream, needed);
    if (read > 0 && clock_is_started(gw->clock)) {
        clock_advance(gw->clock, (double)read / gw->sample_rate);
    }
    SDL_UnlockMutex(gw->mutex);
}

double audio_gateway_get_position(AudioGateway *gw) {
    SDL_LockMutex(gw->mutex);
    double pos = clock_get(gw->clock);
    SDL_UnlockMutex(gw->mutex);
    return pos;
}

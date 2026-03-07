/*
    Bridges the SDL audio callback to the audio clock entity.
    Reads from the AVAudioFifo and advances the MediaClock.
 */

#ifndef AUDIO_GATEWAY_H
#define AUDIO_GATEWAY_H

#include <SDL2/SDL.h>
#include <libavutil/audio_fifo.h>
#include <libavutil/samplefmt.h>
#include <libavutil/channel_layout.h>
#include "media_clock.h"

typedef struct {
    AVAudioFifo   *fifo;
    SDL_mutex     *mutex;
    MediaClock    *clock;        /* pointer into AppContext */
    int            sample_rate;
    int            channels;
} AudioGateway;

/*
 * SDL audio callback: reads PCM from the FIFO,
 * advances the audio clock by the number of samples consumed.
 */
void audio_gateway_callback(void *userdata, Uint8 *stream, int len);

/*
 * Return the current "played" audio position in seconds.
 * Thread-safe (locks the mutex).
 */
double audio_gateway_get_position(AudioGateway *gw);

#endif

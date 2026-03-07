/*
    FFmpeg audio decode thread.
 */

#ifndef AUDIO_DECODER_H
#define AUDIO_DECODER_H

#include "app_context.h"

int  audio_decoder_init(AppContext *ctx);

/* Wait for decode thread + free codec / resampler / FIFO. */
void audio_decoder_cleanup(AppContext *ctx);

#endif

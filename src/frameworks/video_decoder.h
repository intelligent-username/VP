/*
    Threaded video decoding.
    Decodes video packets and enqueues BGRA frames to the FrameQueue.
 */

#ifndef VIDEO_DECODER_H
#define VIDEO_DECODER_H

#include "app_context.h"

/* Open video codec (with multithreading). */
int  video_decoder_init(AppContext *ctx);

/* Start the decode thread. */
int  video_decoder_start(AppContext *ctx);

/* Wait for thread + free codec. */
void video_decoder_cleanup(AppContext *ctx);

#endif

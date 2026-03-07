/*
    FFmpeg demuxer thread.
 */

#ifndef DEMUXER_H
#define DEMUXER_H

#include "app_context.h"

int  demuxer_start(AppContext *ctx);
void demuxer_stop(AppContext *ctx);

#endif

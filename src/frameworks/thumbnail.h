/*
    Extract a representative frame from a video.
    Returns raw RGB24 data for the interface adapter to convert.
 */

 #ifndef THUMBNAIL_H
#define THUMBNAIL_H

#include <stdint.h>

typedef struct {
    uint8_t *data;      /* RGB24 pixel buffer (caller frees) */
    int      width;
    int      height;
} ThumbnailData;

/* Extract a thumbnail from `path`, scaled to `target_w` wide.
 * Height is calculated to maintain aspect ratio.
 * Returns 0 on success. */
int  thumbnail_extract(const char *path, ThumbnailData *out,
                       int target_w);

void thumbnail_free(ThumbnailData *td);

#endif

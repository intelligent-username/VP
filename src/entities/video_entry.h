/*
    A single video file in the library.
 */

#ifndef VIDEO_ENTRY_H
#define VIDEO_ENTRY_H

#define VE_PATH_MAX  512
#define VE_TITLE_MAX 256
#define VE_MAX_ENTRIES 64

typedef struct {
    char path[VE_PATH_MAX];
    char title[VE_TITLE_MAX];
} VideoEntry;

typedef struct {
    VideoEntry entries[VE_MAX_ENTRIES];
    int        count;
} VideoLibrary;

/* Reset library to empty */
void video_library_init(VideoLibrary *lib);

/* Add an entry. Returns 0 on success, -1 if full. */
int  video_library_add(VideoLibrary *lib,
                       const char *path, const char *title);

#endif /* VIDEO_ENTRY_H */

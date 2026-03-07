/*
    Video library operations.
 */

#include "video_entry.h"
#include <string.h>

void video_library_init(VideoLibrary *lib) {
    lib->count = 0;
}

int video_library_add(VideoLibrary *lib,
                      const char *path, const char *title) {
    if (lib->count >= VE_MAX_ENTRIES) return -1;

    VideoEntry *e = &lib->entries[lib->count];
    strncpy(e->path,  path,  VE_PATH_MAX  - 1);
    strncpy(e->title, title, VE_TITLE_MAX - 1);
    e->path[VE_PATH_MAX - 1]   = '\0';
    e->title[VE_TITLE_MAX - 1] = '\0';
    lib->count++;
    return 0;
}

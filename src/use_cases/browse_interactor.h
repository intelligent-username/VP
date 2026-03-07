/*
    Scan a directory for playable videos.
    NOTE: Won't be a directory, but a list of 'recommendations' in the future
 */

#ifndef BROWSE_INTERACTOR_H
#define BROWSE_INTERACTOR_H

#include "video_entry.h"

/* Scan `dir_path` for video files and populate `lib`.
 * Returns number of videos found. */
int browse_scan_directory(const char *dir_path, VideoLibrary *lib);

#endif

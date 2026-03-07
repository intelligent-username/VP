/*
    Scans directory for video files.
    NOTE: in the future it won't be a directory but a dynamically returned list of recommendations.
 */

#include "browse_interactor.h"
#include <dirent.h>
#include <string.h>
#include <stdio.h>

static const char *VIDEO_EXTS[] = {
    ".mp4", ".mkv", ".avi", ".webm", ".mov", ".flv", ".ts", NULL
};

static int has_video_extension(const char *name) {
    const char *dot = strrchr(name, '.');
    if (!dot) return 0;
    for (int i = 0; VIDEO_EXTS[i]; i++) {
        if (strcasecmp(dot, VIDEO_EXTS[i]) == 0) return 1;
    }
    return 0;
}

static void extract_title(const char *filename, char *out, int max) {
    strncpy(out, filename, max - 1);
    out[max - 1] = '\0';
    char *dot = strrchr(out, '.');
    if (dot) *dot = '\0';   /* strip extension */
}

static void build_full_path(const char *dir, const char *name,
                            char *out, int max) {
    snprintf(out, max, "%s/%s", dir, name);
}

int browse_scan_directory(const char *dir_path, VideoLibrary *lib) {
    video_library_init(lib);

    DIR *d = opendir(dir_path);
    if (!d) { perror("opendir"); return 0; }

    struct dirent *ent;
    while ((ent = readdir(d)) != NULL) {
        if (!has_video_extension(ent->d_name)) continue;

        char path[VE_PATH_MAX];
        char title[VE_TITLE_MAX];
        build_full_path(dir_path, ent->d_name, path, VE_PATH_MAX);
        extract_title(ent->d_name, title, VE_TITLE_MAX);
        video_library_add(lib, path, title);
    }
    closedir(d);
    return lib->count;
}

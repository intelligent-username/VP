/*
    Handles search logic for filtering videos.
 */

#ifndef SEARCH_INTERACTOR_H
#define SEARCH_INTERACTOR_H

#include <gtk/gtk.h>
#include "video_entry.h"

typedef struct {
    GtkFlowBox    *flow_box;
    VideoLibrary  *library;
} SearchInteractor;

void search_interactor_init(SearchInteractor *si, GtkFlowBox *flow_box, VideoLibrary *lib);
void search_interactor_on_changed(SearchInteractor *si, const char *text);

#endif
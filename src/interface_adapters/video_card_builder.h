/*
    Video card builder.
    Creates individual video thumbnail cards for the home grid.
 */

#ifndef VIDEO_CARD_BUILDER_H
#define VIDEO_CARD_BUILDER_H

#include <gtk/gtk.h>
#include "video_entry.h"

typedef void (*on_card_selected_callback)(int video_index, void *userdata);

/* Build a single video card widget */
GtkWidget *video_card_builder_create(int video_index,
                                     const VideoEntry *entry,
                                     on_card_selected_callback on_select,
                                     void *userdata);

#endif

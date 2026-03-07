/*
    Renders video frames via Cairo on a GtkDrawingArea.
    Replaces the old SDL-based presenter.
 */

#ifndef VIDEO_PRESENTER_H
#define VIDEO_PRESENTER_H

#include <gtk/gtk.h>
#include "frame_queue.h"

typedef struct {
    GtkWidget *drawing_area;
    GtkWidget *progress_bar;
    GtkWidget *container;       /* VBox holding area + bar */

    /* Current frame being displayed */
    uint8_t   *current_data;
    int        current_w;
    int        current_h;
    int        current_stride;
} VideoPresenter;

/* Build the GTK widgets (drawing area + progress bar). */
void vp_init(VideoPresenter *vp);

/* Update the displayed frame (copies data). */
void vp_set_frame(VideoPresenter *vp, const RGBFrame *f);

/* Update progress bars (0.0 - 1.0). */
void vp_set_progress(VideoPresenter *vp, double pct);

/* Free resources. */
void vp_cleanup(VideoPresenter *vp);

#endif

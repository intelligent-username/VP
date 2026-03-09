/*
    Renders video frames via Cairo on a GtkDrawingArea.
    Replaces the old SDL-based presenter.
 */

#ifndef VIDEO_PRESENTER_H
#define VIDEO_PRESENTER_H

#include <gtk/gtk.h>
#include "frame_queue.h"
#include "playback_mode_interactor.h"
#include "playback_view_manager.h"
#include "playback_input_handler.h"

typedef struct VideoPresenter {
    GtkWidget *drawing_area;
    GtkWidget *progress_bar;
    GtkWidget *container;       /* VBox holding area + bar */

    /* Current frame being displayed */
    uint8_t   *current_data;
    int        current_w;
    int        current_h;
    int        current_stride;

    /* Playback mode management */
    PlaybackModeState       mode_state;
    PlaybackViewManager     view_manager;
    PlaybackInputHandler    input_handler;
} VideoPresenter;

/* Build the GTK widgets (drawing area + progress bar). */
void vp_init(VideoPresenter *vp);

/* Update the displayed frame (copies data). */
void vp_set_frame(VideoPresenter *vp, const RGBFrame *f);

/* Update progress bars (0.0 - 1.0). */
void vp_set_progress(VideoPresenter *vp, double pct);

/* Playback mode control */
void vp_set_fullscreen_mode(VideoPresenter *vp);
void vp_set_minimized_mode(VideoPresenter *vp);
void vp_hide_miniplayer(VideoPresenter *vp);
void vp_on_video_finished(VideoPresenter *vp);

void vp_set_expand_callback(VideoPresenter *vp,
                            playback_action_callback callback,
                            void *userdata);

void vp_set_close_callback(VideoPresenter *vp,
                           playback_action_callback callback,
                           void *userdata);

/* Connect keyboard input handling with custom minimize/fullscreen callbacks */
void vp_connect_input_with_callback(VideoPresenter *vp, GtkWidget *window,
                                    playback_action_callback on_minimize,
                                    void *minimize_userdata,
                                    playback_action_callback on_fullscreen,
                                    void *fullscreen_userdata);

/* Free resources. */
void vp_cleanup(VideoPresenter *vp);

#endif

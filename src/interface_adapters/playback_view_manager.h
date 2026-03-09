/*
    Renders the video in the appropriate container based on playback mode.
    Handles UI transitions between fullscreen and minimized modes.
 */

#ifndef PLAYBACK_VIEW_MANAGER_H
#define PLAYBACK_VIEW_MANAGER_H

#include <gtk/gtk.h>
#include "playback_mode.h"
#include "playback_overlay_handler.h"

typedef void (*playback_view_action_callback)(void *data);

typedef struct {
    GtkWidget *fullscreen_container;   /* host page for fullscreen player */
    GtkWidget *fullscreen_content;     /* host content inside fullscreen overlay */
    GtkWidget *fullscreen_close_button;/* top-left close button on fullscreen */
    GtkWidget *minimized_container;    /* overlay frame */
    GtkWidget *minimized_overlay;      /* overlay to place button on video */
    GtkWidget *minimized_content;      /* host box inside mini frame */
    GtkWidget *expand_button;          /* top-right expand button */
    GtkWidget *close_button;           /* top-left close button */
    GtkWidget *video_container;        /* actual video widget moved between hosts */
    OverlayButtons overlay_buttons;    /* persistent hover callback data */
    PlaybackMode current_mode;

    playback_view_action_callback on_expand;
    void *expand_userdata;
    playback_view_action_callback on_close;
    void *close_userdata;
} PlaybackViewManager;

void playback_view_manager_init(PlaybackViewManager *pvm, GtkWidget *video_container);

void playback_view_manager_set_expand_callback(PlaybackViewManager *pvm,
                                               playback_view_action_callback cb,
                                               void *userdata);

void playback_view_manager_set_close_callback(PlaybackViewManager *pvm,
                                              playback_view_action_callback cb,
                                              void *userdata);

/* Switch to fullscreen view */
void playback_view_manager_show_fullscreen(PlaybackViewManager *pvm, GtkWidget *parent);

/* Switch to minimized view (stays visible alongside other content) */
void playback_view_manager_show_minimized(PlaybackViewManager *pvm, GtkWidget *parent);

/* Hide mini overlay explicitly */
void playback_view_manager_hide_minimized(PlaybackViewManager *pvm);

/* Get current viewing mode */
PlaybackMode playback_view_manager_get_mode(PlaybackViewManager *pvm);

/* Clean up resources */
void playback_view_manager_cleanup(PlaybackViewManager *pvm);

#endif

/*
    Handles keyboard input for playback controls (Escape to minimize, etc.)
 */

#ifndef PLAYBACK_INPUT_HANDLER_H
#define PLAYBACK_INPUT_HANDLER_H

#include <gtk/gtk.h>

typedef void (*playback_action_callback)(void *data);

typedef struct {
    playback_action_callback on_minimize;
    void *minimize_userdata;
    playback_action_callback on_fullscreen;
    void *fullscreen_userdata;
} PlaybackInputHandler;

void playback_input_handler_init(PlaybackInputHandler *pih, 
                                 playback_action_callback on_minimize,
                                 void *minimize_userdata,
                                 playback_action_callback on_fullscreen,
                                 void *fullscreen_userdata);

/* Connect key press event to a window */
void playback_input_handler_connect(PlaybackInputHandler *pih, GtkWidget *window);

#endif

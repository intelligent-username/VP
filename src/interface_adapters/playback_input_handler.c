/*
    Playback Input Handler
 */

#include "playback_input_handler.h"

static gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer data) {
    (void)widget;
    PlaybackInputHandler *pih = (PlaybackInputHandler *)data;
    
    /* Escape key (GDK_KEY_Escape = 0xFF1B) */
    if (event->keyval == GDK_KEY_Escape) {
        if (pih->on_minimize) {
            pih->on_minimize(pih->minimize_userdata);
        }
        return TRUE;  /* Consume the event */
    }

    if (event->keyval == GDK_KEY_f || event->keyval == GDK_KEY_F) {
        if (pih->on_fullscreen) {
            pih->on_fullscreen(pih->fullscreen_userdata);
        }
        return TRUE;
    }
    
    return FALSE;  /* Let other handlers process it */
}

void playback_input_handler_init(PlaybackInputHandler *pih,
                                 playback_action_callback on_minimize,
                                 void *minimize_userdata,
                                 playback_action_callback on_fullscreen,
                                 void *fullscreen_userdata) {
    pih->on_minimize = on_minimize;
    pih->minimize_userdata = minimize_userdata;
    pih->on_fullscreen = on_fullscreen;
    pih->fullscreen_userdata = fullscreen_userdata;
}

void playback_input_handler_connect(PlaybackInputHandler *pih, GtkWidget *window) {
    g_signal_connect(window, "key-press-event", G_CALLBACK(on_key_press), pih);
}

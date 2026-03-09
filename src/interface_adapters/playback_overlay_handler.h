/*
    Overlay hover and button visibility manager.
    Handles shared hover logic for all overlay buttons.
 */

#ifndef PLAYBACK_OVERLAY_HANDLER_H
#define PLAYBACK_OVERLAY_HANDLER_H

#include <gtk/gtk.h>

typedef struct {
    GtkWidget *expand_button;
    GtkWidget *close_button;
    GtkWidget *fullscreen_close_button;
} OverlayButtons;

/* Set opacity for all overlay buttons together */
void playback_overlay_handler_set_button_opacity(OverlayButtons *buttons, double opacity);

/* Connect hover events to an overlay container */
void playback_overlay_handler_setup_hover(GtkWidget *overlay_container,
                                          OverlayButtons *buttons);

#endif

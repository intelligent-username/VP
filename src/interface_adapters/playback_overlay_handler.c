/*
    Overlay hover and button visibility manager.
 */

#include "playback_overlay_handler.h"

void playback_overlay_handler_set_button_opacity(OverlayButtons *buttons, double opacity) {
    if (buttons->expand_button) gtk_widget_set_opacity(buttons->expand_button, opacity);
    if (buttons->close_button) gtk_widget_set_opacity(buttons->close_button, opacity);
    if (buttons->fullscreen_close_button) gtk_widget_set_opacity(buttons->fullscreen_close_button, opacity);
}

static gboolean on_overlay_enter(GtkWidget *widget, GdkEventCrossing *event, gpointer data) {
    (void)widget;
    (void)event;
    OverlayButtons *buttons = (OverlayButtons *)data;
    playback_overlay_handler_set_button_opacity(buttons, 0.9);
    return FALSE;
}

static gboolean on_overlay_leave(GtkWidget *widget, GdkEventCrossing *event, gpointer data) {
    (void)widget;
    (void)event;
    OverlayButtons *buttons = (OverlayButtons *)data;
    playback_overlay_handler_set_button_opacity(buttons, 0.0);
    return FALSE;
}

void playback_overlay_handler_setup_hover(GtkWidget *overlay_container, OverlayButtons *buttons) {
    gtk_widget_add_events(overlay_container, GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK);
    g_signal_connect(overlay_container, "enter-notify-event", G_CALLBACK(on_overlay_enter), buttons);
    g_signal_connect(overlay_container, "leave-notify-event", G_CALLBACK(on_overlay_leave), buttons);
}

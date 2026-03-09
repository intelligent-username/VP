/*
    Playback View Manager
 */

#include "playback_view_manager.h"

static void reparent_widget(GtkWidget *child, GtkWidget *new_parent) {
    if (!child || !new_parent) return;

    GtkWidget *old_parent = gtk_widget_get_parent(child);
    if (old_parent == new_parent) return;

    g_object_ref(child);
    if (old_parent) {
        gtk_container_remove(GTK_CONTAINER(old_parent), child);
    }
    gtk_container_add(GTK_CONTAINER(new_parent), child);
    g_object_unref(child);
}

static void on_expand_clicked(GtkButton *button, gpointer data) {
    (void)button;
    PlaybackViewManager *pvm = (PlaybackViewManager *)data;
    if (pvm->on_expand) {
        pvm->on_expand(pvm->expand_userdata);
    }
}

static void on_close_clicked(GtkButton *button, gpointer data) {
    (void)button;
    PlaybackViewManager *pvm = (PlaybackViewManager *)data;
    if (pvm->on_close) {
        pvm->on_close(pvm->close_userdata);
    }
}

void playback_view_manager_init(PlaybackViewManager *pvm, GtkWidget *video_container) {
    pvm->video_container = video_container;
    pvm->current_mode = PLAYBACK_MODE_FULLSCREEN;
    pvm->on_expand = NULL;
    pvm->expand_userdata = NULL;
    pvm->expand_button = NULL;
    pvm->on_close = NULL;
    pvm->close_userdata = NULL;
    pvm->close_button = NULL;
    pvm->fullscreen_content = NULL;
    pvm->fullscreen_close_button = NULL;
    pvm->overlay_buttons.expand_button = NULL;
    pvm->overlay_buttons.close_button = NULL;
    pvm->overlay_buttons.fullscreen_close_button = NULL;

    pvm->fullscreen_container = gtk_overlay_new();
    pvm->fullscreen_content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(pvm->fullscreen_container), pvm->fullscreen_content);
    gtk_box_pack_start(GTK_BOX(pvm->fullscreen_content),
                       pvm->video_container, TRUE, TRUE, 0);

    pvm->fullscreen_close_button = gtk_button_new_with_label("✕");
    gtk_widget_set_halign(pvm->fullscreen_close_button, GTK_ALIGN_START);
    gtk_widget_set_valign(pvm->fullscreen_close_button, GTK_ALIGN_START);
    gtk_widget_set_margin_top(pvm->fullscreen_close_button, 10);
    gtk_widget_set_margin_start(pvm->fullscreen_close_button, 10);
    gtk_widget_set_opacity(pvm->fullscreen_close_button, 0.0);
    g_signal_connect(pvm->fullscreen_close_button, "clicked", G_CALLBACK(on_close_clicked), pvm);
    gtk_overlay_add_overlay(GTK_OVERLAY(pvm->fullscreen_container), pvm->fullscreen_close_button);

    pvm->minimized_container = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(pvm->minimized_container), GTK_SHADOW_IN);
    gtk_widget_set_size_request(pvm->minimized_container, 320, 180);

    gtk_widget_set_margin_start(pvm->minimized_container, 12);
    gtk_widget_set_margin_end(pvm->minimized_container, 12);
    gtk_widget_set_margin_bottom(pvm->minimized_container, 12);

    pvm->minimized_overlay = gtk_overlay_new();
    pvm->minimized_content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(pvm->minimized_overlay), pvm->minimized_content);

    pvm->expand_button = gtk_button_new_with_label("⤢");
    gtk_widget_set_halign(pvm->expand_button, GTK_ALIGN_END);
    gtk_widget_set_valign(pvm->expand_button, GTK_ALIGN_START);
    gtk_widget_set_margin_top(pvm->expand_button, 6);
    gtk_widget_set_margin_end(pvm->expand_button, 6);
    gtk_widget_set_opacity(pvm->expand_button, 0.0);
    g_signal_connect(pvm->expand_button, "clicked", G_CALLBACK(on_expand_clicked), pvm);
    gtk_overlay_add_overlay(GTK_OVERLAY(pvm->minimized_overlay), pvm->expand_button);

    pvm->close_button = gtk_button_new_with_label("✕");
    gtk_widget_set_halign(pvm->close_button, GTK_ALIGN_START);
    gtk_widget_set_valign(pvm->close_button, GTK_ALIGN_START);
    gtk_widget_set_margin_top(pvm->close_button, 6);
    gtk_widget_set_margin_start(pvm->close_button, 6);
    gtk_widget_set_opacity(pvm->close_button, 0.0);
    g_signal_connect(pvm->close_button, "clicked", G_CALLBACK(on_close_clicked), pvm);
    gtk_overlay_add_overlay(GTK_OVERLAY(pvm->minimized_overlay), pvm->close_button);

    pvm->overlay_buttons.expand_button = pvm->expand_button;
    pvm->overlay_buttons.close_button = pvm->close_button;
    pvm->overlay_buttons.fullscreen_close_button = pvm->fullscreen_close_button;

    playback_overlay_handler_setup_hover(pvm->minimized_overlay, &pvm->overlay_buttons);
    playback_overlay_handler_setup_hover(pvm->fullscreen_container, &pvm->overlay_buttons);

    gtk_container_add(GTK_CONTAINER(pvm->minimized_container), pvm->minimized_overlay);
    gtk_widget_set_no_show_all(pvm->minimized_container, TRUE);

    gtk_widget_hide(pvm->minimized_container);
}

void playback_view_manager_set_expand_callback(PlaybackViewManager *pvm,
                                               playback_view_action_callback cb,
                                               void *userdata) {
    pvm->on_expand = cb;
    pvm->expand_userdata = userdata;
}

void playback_view_manager_set_close_callback(PlaybackViewManager *pvm,
                                              playback_view_action_callback cb,
                                              void *userdata) {
    pvm->on_close = cb;
    pvm->close_userdata = userdata;
}

void playback_view_manager_show_fullscreen(PlaybackViewManager *pvm, GtkWidget *parent) {
    (void)parent;
    pvm->current_mode = PLAYBACK_MODE_FULLSCREEN;

    reparent_widget(pvm->video_container, pvm->fullscreen_content);
    gtk_widget_show_all(pvm->fullscreen_container);
    gtk_widget_hide(pvm->minimized_container);
}

void playback_view_manager_show_minimized(PlaybackViewManager *pvm, GtkWidget *parent) {
    (void)parent;
    pvm->current_mode = PLAYBACK_MODE_MINIMIZED;

    reparent_widget(pvm->video_container, pvm->minimized_content);
    gtk_widget_show_all(pvm->minimized_overlay);
    gtk_widget_show(pvm->minimized_container);
}

void playback_view_manager_hide_minimized(PlaybackViewManager *pvm) {
    if (!pvm) return;
    if (pvm->minimized_container) {
        gtk_widget_hide(pvm->minimized_container);
    }
}

PlaybackMode playback_view_manager_get_mode(PlaybackViewManager *pvm) {
    return pvm->current_mode;
}

void playback_view_manager_cleanup(PlaybackViewManager *pvm) {
    pvm->video_container = NULL;
    pvm->fullscreen_content = NULL;
    pvm->fullscreen_close_button = NULL;
    pvm->minimized_overlay = NULL;
    pvm->minimized_content = NULL;
    pvm->expand_button = NULL;
    pvm->close_button = NULL;
    pvm->on_expand = NULL;
    pvm->expand_userdata = NULL;
    pvm->on_close = NULL;
    pvm->close_userdata = NULL;
    pvm->overlay_buttons.expand_button = NULL;
    pvm->overlay_buttons.close_button = NULL;
    pvm->overlay_buttons.fullscreen_close_button = NULL;
}

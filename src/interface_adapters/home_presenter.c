/*
    Window with GtkStack navigation.
 */

#include "home_presenter.h"
#include "search_interactor.h"
#include "search_bar.h"
#include "home_presenter_styling.h"
#include "video_card_builder.h"
#include <string.h>

static gboolean on_scroll_button_press(GtkWidget *widget, GdkEventButton *event, gpointer data) {
    HomePresenter *hp = (HomePresenter *)data;
    if (gtk_widget_has_focus(hp->search_entry)) {
        gtk_window_set_focus(GTK_WINDOW(hp->window), NULL);
    }
    return FALSE;
}

static void on_search_changed(GtkEntry *entry, gpointer data) {
    HomePresenter *hp = (HomePresenter *)data;
    const char *text = gtk_entry_get_text(entry);
    search_interactor_on_changed(&hp->search_interactor, text);
}

static void on_card_selected(int video_index, void *userdata) {
    HomePresenter *hp = (HomePresenter *)userdata;
    const char *path = hp->library->entries[video_index].path;
    strncpy(hp->selected_path, path, VE_PATH_MAX - 1);
    hp->selected_path[VE_PATH_MAX - 1] = '\0';
    hp->has_selection = 1;

    if (playback_mode_get_current(&hp->vp.mode_state) != PLAYBACK_MODE_MINIMIZED) {
        home_presenter_show_player(hp);
    }
}

/* ---- Create search bar and video flow box ---- */

static GtkWidget *build_search_bar_widget(HomePresenter *hp) {
    GtkWidget *search = create_search_bar();
    hp->search_entry = search;
    g_signal_connect(search, "changed", G_CALLBACK(on_search_changed), hp);
    return search;
}

static GtkWidget *create_video_flow_box(HomePresenter *hp) {
    GtkWidget *flow = gtk_flow_box_new();
    gtk_flow_box_set_homogeneous(GTK_FLOW_BOX(flow), FALSE);
    gtk_flow_box_set_max_children_per_line(GTK_FLOW_BOX(flow), 6);
    gtk_flow_box_set_row_spacing(GTK_FLOW_BOX(flow), 12);
    gtk_flow_box_set_column_spacing(GTK_FLOW_BOX(flow), 12);
    gtk_flow_box_set_selection_mode(GTK_FLOW_BOX(flow), GTK_SELECTION_NONE);
    gtk_widget_set_margin_start(flow, 16);
    gtk_widget_set_margin_end(flow, 16);

    hp->flow_box = flow;

    for (int i = 0; i < hp->library->count; i++) {
        GtkWidget *card = video_card_builder_create(i, &hp->library->entries[i],
                                                     on_card_selected, hp);
        gtk_flow_box_insert(GTK_FLOW_BOX(flow), card, -1);
    }

    return flow;
}

/* ---- Build home page ---- */

static GtkWidget *build_home_page(HomePresenter *hp) {
    /* Use Overlay to allow minimized player to float over home content */
    GtkWidget *overlay = gtk_overlay_new();
    hp->home_page_overlay = overlay;

    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(scroll), 400);

    // Connect button press to remove focus from search bar when clicking elsewhere
    g_signal_connect(scroll, "button-press-event", G_CALLBACK(on_scroll_button_press), hp);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    GtkWidget *hdr = gtk_label_new("ViewPort");
    gtk_style_context_add_class(gtk_widget_get_style_context(hdr), "header-label");
    gtk_widget_set_halign(hdr, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(vbox), hdr, FALSE, FALSE, 0);

    GtkWidget *search = build_search_bar_widget(hp);
    gtk_widget_set_margin_bottom(search, 16); // padding BELOW the search bar
    gtk_box_pack_start(GTK_BOX(vbox), search, FALSE, FALSE, 0);

    GtkWidget *flow = create_video_flow_box(hp);
    gtk_box_pack_start(GTK_BOX(vbox), flow, FALSE, FALSE, 0);

    gtk_container_add(GTK_CONTAINER(scroll), vbox);
    gtk_container_add(GTK_CONTAINER(overlay), scroll);

    return overlay;
}

/* ---- Public API ---- */

void home_presenter_init(HomePresenter *hp, VideoLibrary *lib) {
    memset(hp, 0, sizeof(*hp));
    hp->library = lib;
    home_presenter_styling_init();

    hp->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(hp->window), "ViewPort");
    gtk_window_set_default_size(GTK_WINDOW(hp->window), 960, 600);
    gtk_window_set_position(GTK_WINDOW(hp->window), GTK_WIN_POS_CENTER);
    g_signal_connect(hp->window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    hp->stack = gtk_stack_new();
    gtk_stack_set_transition_type(GTK_STACK(hp->stack),
                                  GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);
    gtk_stack_set_transition_duration(GTK_STACK(hp->stack), 300);

    hp->home_page = build_home_page(hp);
    gtk_stack_add_named(GTK_STACK(hp->stack), hp->home_page, "home");

    // Initialize search interactor after flow_box is created
    search_interactor_init(&hp->search_interactor, GTK_FLOW_BOX(hp->flow_box), hp->library);

    vp_init(&hp->vp);
    gtk_stack_add_named(GTK_STACK(hp->stack),
                        hp->vp.view_manager.fullscreen_container,
                        "player");

    /* Add minimized player as overlay on home page */
    gtk_overlay_add_overlay(GTK_OVERLAY(hp->home_page_overlay), 
                            hp->vp.view_manager.minimized_container);
    gtk_widget_set_halign(hp->vp.view_manager.minimized_container, GTK_ALIGN_END);
    gtk_widget_set_valign(hp->vp.view_manager.minimized_container, GTK_ALIGN_END);
    vp_hide_miniplayer(&hp->vp);

    gtk_container_add(GTK_CONTAINER(hp->window), hp->stack);
    gtk_widget_show_all(hp->window);
}

void home_presenter_show_home(HomePresenter *hp) {
    gtk_stack_set_visible_child_name(GTK_STACK(hp->stack), "home");
}

void home_presenter_show_player(HomePresenter *hp) {
    gtk_stack_set_visible_child_name(GTK_STACK(hp->stack), "player");
}

const char *home_presenter_get_selection(HomePresenter *hp) {
    return hp->has_selection ? hp->selected_path : NULL;
}

void home_presenter_clear_selection(HomePresenter *hp) {
    hp->has_selection = 0;
    hp->selected_path[0] = '\0';
}

void home_presenter_destroy(HomePresenter *hp) {
    vp_cleanup(&hp->vp);
}

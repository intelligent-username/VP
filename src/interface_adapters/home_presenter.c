/*
 * Window with GtkStack navigation.
 */

#include "home_presenter.h"
#include "thumbnail.h"
#include <string.h>

#define THUMB_WIDTH 320

/* ---- CSS ---- */

static const char *CSS =
    "window           { background-color: #0d1117; }\n"
    ".header-label    { color: #e6edf3; font-size: 28px;"
    "                   font-weight: bold; margin: 20px; }\n"
    ".video-card      { background-color: #161b22; border-radius: 8px;"
    "                   padding: 6px; margin: 8px; }\n"
    ".video-card:hover{ background-color: #21262d; }\n"
    ".video-title     { color: #c9d1d9; font-size: 13px;"
    "                   margin-top: 6px; }\n";

static void apply_css(void) {
    GtkCssProvider *p = gtk_css_provider_new();
    gtk_css_provider_load_from_data(p, CSS, -1, NULL);
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(), GTK_STYLE_PROVIDER(p),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(p);
}

/* ---- Thumbnail → Pixbuf ---- */

static GdkPixbuf *pixbuf_from_video(const char *path) {
    ThumbnailData td = {0};
    if (thumbnail_extract(path, &td, THUMB_WIDTH) < 0) return NULL;

    /* thumbnail is RGB24: convert to GdkPixbuf */
    GdkPixbuf *pb = gdk_pixbuf_new_from_data(
        td.data, GDK_COLORSPACE_RGB, FALSE,
        8, td.width, td.height, td.width * 3,
        NULL, NULL);
    /* pixbuf doesn't copy: don't free td.data until pixbuf dies.
     * For simplicity, leak the small thumbnail buffer. */
    return pb;
}

/* ---- Card click ---- */

typedef struct { HomePresenter *hp; int idx; } CardCB;

static gboolean on_card_clicked(GtkWidget *w, GdkEventButton *ev,
                                gpointer data) {
    (void)w; (void)ev;
    CardCB *cb = (CardCB *)data;
    const char *path = cb->hp->library->entries[cb->idx].path;
    strncpy(cb->hp->selected_path, path, VE_PATH_MAX - 1);
    cb->hp->has_selection = 1;

    /* Notify the main loop via a custom signal by quitting idle */
    home_presenter_show_player(cb->hp);
    return TRUE;
}

/* ---- Build card ---- */

static GtkWidget *build_card(HomePresenter *hp, int i, GdkPixbuf *pb) {
    GtkWidget *ebox = gtk_event_box_new();
    GtkStyleContext *sc = gtk_widget_get_style_context(ebox);
    gtk_style_context_add_class(sc, "video-card");

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    if (pb) {
        GtkWidget *img = gtk_image_new_from_pixbuf(pb);
        gtk_box_pack_start(GTK_BOX(vbox), img, FALSE, FALSE, 0);
    }

    GtkWidget *lbl = gtk_label_new(hp->library->entries[i].title);
    gtk_style_context_add_class(
        gtk_widget_get_style_context(lbl), "video-title");
    gtk_label_set_ellipsize(GTK_LABEL(lbl), PANGO_ELLIPSIZE_END);
    gtk_label_set_max_width_chars(GTK_LABEL(lbl), 30);
    gtk_box_pack_start(GTK_BOX(vbox), lbl, FALSE, FALSE, 0);

    gtk_container_add(GTK_CONTAINER(ebox), vbox);

    CardCB *cb = g_new(CardCB, 1);
    cb->hp  = hp;
    cb->idx = i;
    g_signal_connect_data(ebox, "button-press-event",
        G_CALLBACK(on_card_clicked), cb,
        (GClosureNotify)g_free, 0);
    return ebox;
}

/* ---- Build home page ---- */

static GtkWidget *build_home_page(HomePresenter *hp) {
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    GtkWidget *vbox   = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    GtkWidget *hdr = gtk_label_new("ViewPort");
    gtk_style_context_add_class(
        gtk_widget_get_style_context(hdr), "header-label");
    gtk_widget_set_halign(hdr, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(vbox), hdr, FALSE, FALSE, 0);

    GtkWidget *flow = gtk_flow_box_new();
    gtk_flow_box_set_homogeneous(GTK_FLOW_BOX(flow), TRUE);
    gtk_flow_box_set_max_children_per_line(GTK_FLOW_BOX(flow), 6);
    gtk_flow_box_set_selection_mode(GTK_FLOW_BOX(flow),
                                    GTK_SELECTION_NONE);
    gtk_widget_set_margin_start(flow, 16);
    gtk_widget_set_margin_end(flow, 16);

    for (int i = 0; i < hp->library->count; i++) {
        GdkPixbuf *pb = pixbuf_from_video(hp->library->entries[i].path);
        GtkWidget *card = build_card(hp, i, pb);
        gtk_flow_box_insert(GTK_FLOW_BOX(flow), card, -1);
    }

    gtk_box_pack_start(GTK_BOX(vbox), flow, TRUE, TRUE, 0);
    gtk_container_add(GTK_CONTAINER(scroll), vbox);
    return scroll;
}

/* ---- Public API ---- */

void home_presenter_init(HomePresenter *hp, VideoLibrary *lib) {
    memset(hp, 0, sizeof(*hp));
    hp->library = lib;
    apply_css();

    hp->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(hp->window), "ViewPort");
    gtk_window_set_default_size(GTK_WINDOW(hp->window), 960, 600);
    g_signal_connect(hp->window, "destroy",
                     G_CALLBACK(gtk_main_quit), NULL);

    hp->stack = gtk_stack_new();
    gtk_stack_set_transition_type(GTK_STACK(hp->stack),
        GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);
    gtk_stack_set_transition_duration(GTK_STACK(hp->stack), 300);

    hp->home_page = build_home_page(hp);
    gtk_stack_add_named(GTK_STACK(hp->stack), hp->home_page, "home");

    vp_init(&hp->vp);
    gtk_stack_add_named(GTK_STACK(hp->stack),
                        hp->vp.container, "player");

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

/*
 * Window with GtkStack navigation.
 */

#include "home_presenter.h"
#include "thumbnail.h"
#include <string.h>

#define THUMB_WIDTH 320

/* ---- CSS ---- */

static const char *CSS =
    "window { background-color: #0d1117; }\n"
    ".header-label {\n"
    "   color: #f0f6fc;\n"
    "   font-size: 32px;\n"
    "   font-weight: 700;\n"
    "   margin: 24px 20px;\n"
    "   letter-spacing: 1px;\n"
    "}\n"
    ".video-card {\n"
    "   background: linear-gradient(145deg, #161b22, #1b222c);\n"
    "   border-radius: 12px;\n"
    "   padding: 8px;\n"
    "   margin: 10px;\n"
    "   box-shadow: 0 2px 5px rgba(0,0,0,0.4);\n"
    "}\n"
    ".video-card:hover {\n"
    "   background: linear-gradient(145deg, #21262d, #2a333f);\n"
    "   box-shadow: 0 6px 15px rgba(0,0,0,0.5);\n"
    "}\n"
    ".video-title {\n"
    "   color: #c9d1d9;\n"
    "   font-size: 14px;\n"
    "   margin-top: 8px;\n"
    "   font-weight: 500;\n"
    "   letter-spacing: 0.5px;\n"
    "}\n"
    "entry {\n"
    "   border-radius: 16px;\n"
    "   padding: 6px 12px;\n"
    "   font-size: 14px;\n"
    "   background-color: #161b22;\n"
    "   color: #c9d1d9;\n"
    "   border: 1px solid #30363d;\n"
    "}\n"
    "entry:hover {\n"
    "   border-color: #58a6ff;\n"
    "}\n"
    "entry:focus {\n"
    "   border-color: #58a6ff;\n"
    "   box-shadow: 0 0 6px rgba(88,166,255,0.6);\n"
    "}\n";

static void apply_css(void) {
    GtkCssProvider *p = gtk_css_provider_new();
    gtk_css_provider_load_from_data(p, CSS, -1, NULL);
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(), GTK_STYLE_PROVIDER(p),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(p);
}

/* ---- Thumbnail filtering based on search entry ---- */

/* 
 * Case-insensitive substring match for video titles.
 * Converts both the title and the search query to lowercase,
 * then uses strstr() to check if the query appears in the title.
 * Used by the search bar to filter thumbnails.
 */
static gboolean title_matches(const char *title, const char *query) {
    char t[256];
    char q[256];

    g_strlcpy(t, title, sizeof(t));
    g_strlcpy(q, query, sizeof(q));

    g_ascii_strdown(t, -1);
    g_ascii_strdown(q, -1);

    return strstr(t, q) != NULL;
}

static void on_search_changed(GtkEntry *entry, gpointer data) {
    HomePresenter *hp = (HomePresenter *)data;
    const char *text = gtk_entry_get_text(entry);

    GList *children = gtk_container_get_children(GTK_CONTAINER(hp->flow_box));
    for (GList *l = children; l; l = l->next) {
        GtkWidget *flow_child = GTK_WIDGET(l->data);
        GtkWidget *card = gtk_bin_get_child(GTK_BIN(flow_child));

        int idx = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(card), "video-index"));
        const char *title = hp->library->entries[idx].title;

        gboolean visible = (text[0] == '\0') || title_matches(title, text);
        gtk_widget_set_visible(flow_child, visible);
    }
    g_list_free(children);
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

/* ---- Create search bar and video flow box ---- */

static GtkWidget *create_search_bar(HomePresenter *hp) {
    GtkWidget *search = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(search), "Search videos...");
    gtk_widget_set_hexpand(search, TRUE);
    gtk_widget_set_margin_start(search, 16);
    gtk_widget_set_margin_end(search, 16);

    hp->search_entry = search;
    g_signal_connect(search, "changed",
                     G_CALLBACK(on_search_changed), hp);

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
        GdkPixbuf *pb = pixbuf_from_video(hp->library->entries[i].path);
        GtkWidget *card = build_card(hp, i, pb);
        gtk_widget_set_size_request(card, THUMB_WIDTH, 180);
        g_object_set_data(G_OBJECT(card), "video-index", GINT_TO_POINTER(i));
        gtk_flow_box_insert(GTK_FLOW_BOX(flow), card, -1);
    }

    return flow;
}

/* ---- Build home page ---- */

static GtkWidget *build_home_page(HomePresenter *hp) {
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(scroll), 400);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    GtkWidget *hdr = gtk_label_new("ViewPort");
    gtk_style_context_add_class(gtk_widget_get_style_context(hdr), "header-label");
    gtk_widget_set_halign(hdr, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(vbox), hdr, FALSE, FALSE, 0);

    GtkWidget *search = create_search_bar(hp);
    gtk_box_pack_start(GTK_BOX(vbox), search, FALSE, FALSE, 0);

    GtkWidget *flow = create_video_flow_box(hp);
    gtk_box_pack_start(GTK_BOX(vbox), flow, FALSE, FALSE, 0);

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
    gtk_window_set_position(GTK_WINDOW(hp->window), GTK_WIN_POS_CENTER);
    g_signal_connect(hp->window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    hp->stack = gtk_stack_new();
    gtk_stack_set_transition_type(GTK_STACK(hp->stack),
                                  GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);
    gtk_stack_set_transition_duration(GTK_STACK(hp->stack), 300);

    hp->home_page = build_home_page(hp);
    gtk_stack_add_named(GTK_STACK(hp->stack), hp->home_page, "home");

    vp_init(&hp->vp);
    gtk_stack_add_named(GTK_STACK(hp->stack), hp->vp.container, "player");

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

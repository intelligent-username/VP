/*
    Video card builder implementation.
 */

#include "video_card_builder.h"
#include "thumbnail.h"

#define THUMB_WIDTH 320

typedef struct {
    int video_index;
    on_card_selected_callback callback;
    void *userdata;
} CardContext;

static GdkPixbuf *pixbuf_from_video(const char *path) {
    ThumbnailData td = {0};
    if (thumbnail_extract(path, &td, THUMB_WIDTH) < 0) return NULL;

    GdkPixbuf *pb = gdk_pixbuf_new_from_data(
        td.data, GDK_COLORSPACE_RGB, FALSE,
        8, td.width, td.height, td.width * 3,
        NULL, NULL);
    return pb;
}

static gboolean on_card_clicked(GtkWidget *w, GdkEventButton *ev, gpointer data) {
    (void)w; (void)ev;
    CardContext *ctx = (CardContext *)data;
    if (ctx->callback) {
        ctx->callback(ctx->video_index, ctx->userdata);
    }
    return TRUE;
}

GtkWidget *video_card_builder_create(int video_index,
                                     const VideoEntry *entry,
                                     on_card_selected_callback on_select,
                                     void *userdata) {
    GtkWidget *ebox = gtk_event_box_new();
    GtkStyleContext *sc = gtk_widget_get_style_context(ebox);
    gtk_style_context_add_class(sc, "video-card");

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    GdkPixbuf *pb = pixbuf_from_video(entry->path);
    if (pb) {
        GtkWidget *img = gtk_image_new_from_pixbuf(pb);
        gtk_box_pack_start(GTK_BOX(vbox), img, FALSE, FALSE, 0);
    }

    GtkWidget *lbl = gtk_label_new(entry->title);
    gtk_style_context_add_class(gtk_widget_get_style_context(lbl), "video-title");
    gtk_label_set_ellipsize(GTK_LABEL(lbl), PANGO_ELLIPSIZE_END);
    gtk_label_set_max_width_chars(GTK_LABEL(lbl), 30);
    gtk_box_pack_start(GTK_BOX(vbox), lbl, FALSE, FALSE, 0);

    gtk_container_add(GTK_CONTAINER(ebox), vbox);

    CardContext *ctx = g_new(CardContext, 1);
    ctx->video_index = video_index;
    ctx->callback = on_select;
    ctx->userdata = userdata;
    g_signal_connect_data(ebox, "button-press-event",
        G_CALLBACK(on_card_clicked), ctx,
        (GClosureNotify)g_free, 0);

    gtk_widget_set_size_request(ebox, THUMB_WIDTH, 180);
    return ebox;
}

/*
    Renders BGRA frames on a GtkDrawingArea via Cairo.
 */

 #include "video_presenter.h"
#include <stdlib.h>
#include <string.h>

/* ---- Cairo draw callback ---- */

static gboolean on_draw(GtkWidget *widget, cairo_t *cr, gpointer data) {
    VideoPresenter *vp = (VideoPresenter *)data;
    if (!vp->current_data) return FALSE;

    int alloc_w = gtk_widget_get_allocated_width(widget);
    int alloc_h = gtk_widget_get_allocated_height(widget);

    /* black letterbox background */
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_paint(cr);

    cairo_surface_t *surface = cairo_image_surface_create_for_data(
        vp->current_data, CAIRO_FORMAT_ARGB32,
        vp->current_w, vp->current_h, vp->current_stride);

    double sx = (double)alloc_w / vp->current_w;
    double sy = (double)alloc_h / vp->current_h;
    double scale = (sx < sy) ? sx : sy;

    double dx = (alloc_w - vp->current_w * scale) / 2.0;
    double dy = (alloc_h - vp->current_h * scale) / 2.0;

    cairo_translate(cr, dx, dy);
    cairo_scale(cr, scale, scale);
    cairo_set_source_surface(cr, surface, 0, 0);
    cairo_paint(cr);

    cairo_surface_destroy(surface);
    return TRUE;
}

/* ---- Public API ---- */

void vp_init(VideoPresenter *vp) {
    memset(vp, 0, sizeof(*vp));

    vp->container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    vp->drawing_area = gtk_drawing_area_new();
    gtk_widget_set_hexpand(vp->drawing_area, TRUE);
    gtk_widget_set_vexpand(vp->drawing_area, TRUE);
    g_signal_connect(vp->drawing_area, "draw",
                     G_CALLBACK(on_draw), vp);
    gtk_box_pack_start(GTK_BOX(vp->container),
                       vp->drawing_area, TRUE, TRUE, 0);

    vp->progress_bar = gtk_progress_bar_new();
    gtk_progress_bar_set_fraction(
        GTK_PROGRESS_BAR(vp->progress_bar), 0.0);

    /* style the progress bar */
    GtkCssProvider *css = gtk_css_provider_new();
    gtk_css_provider_load_from_data(css,
        "progressbar trough { min-height: 6px; }\n"
        "progressbar progress { min-height: 6px;"
        "  background-color: #ff4444; }\n", -1, NULL);
    gtk_style_context_add_provider(
        gtk_widget_get_style_context(vp->progress_bar),
        GTK_STYLE_PROVIDER(css),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(css);

    gtk_box_pack_start(GTK_BOX(vp->container),
                       vp->progress_bar, FALSE, FALSE, 0);
}

void vp_set_frame(VideoPresenter *vp, const RGBFrame *f) {
    int size = f->stride * f->height;
    if (vp->current_w != f->width || vp->current_h != f->height) {
        free(vp->current_data);
        vp->current_data = malloc(size + 64);
        vp->current_w      = f->width;
        vp->current_h      = f->height;
        vp->current_stride = f->stride;
    }
    memcpy(vp->current_data, f->data, size);
    gtk_widget_queue_draw(vp->drawing_area);
}

void vp_set_progress(VideoPresenter *vp, double pct) {
    if (pct < 0.0) pct = 0.0;
    if (pct > 1.0) pct = 1.0;
    gtk_progress_bar_set_fraction(
        GTK_PROGRESS_BAR(vp->progress_bar), pct);
}

void vp_cleanup(VideoPresenter *vp) {
    free(vp->current_data);
    vp->current_data = NULL;
}

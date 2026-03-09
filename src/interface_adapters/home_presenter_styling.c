/*
    Styling module for home presenter.
 */

#include "home_presenter_styling.h"
#include <gtk/gtk.h>

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

void home_presenter_styling_init(void) {
    GtkCssProvider *p = gtk_css_provider_new();
    gtk_css_provider_load_from_data(p, CSS, -1, NULL);
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(), GTK_STYLE_PROVIDER(p),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(p);
}

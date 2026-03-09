/*
    Creates the search bar widget.
 */

#include "search_bar.h"

GtkWidget *create_search_bar(void) {
    GtkWidget *search = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(search), "Search videos...");
    gtk_widget_set_hexpand(search, TRUE);
    gtk_widget_set_margin_start(search, 16);
    gtk_widget_set_margin_end(search, 16);

    return search;
}
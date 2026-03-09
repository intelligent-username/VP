/*
    Search Interactor: Handles search logic for filtering videos.
 */

#include "search_interactor.h"
#include <string.h>

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

void search_interactor_init(SearchInteractor *si, GtkFlowBox *flow_box, VideoLibrary *lib) {
    si->flow_box = flow_box;
    si->library = lib;
}

void search_interactor_on_changed(SearchInteractor *si, const char *text) {
    GList *children = gtk_container_get_children(GTK_CONTAINER(si->flow_box));
    for (GList *l = children; l; l = l->next) {
        GtkWidget *flow_child = GTK_WIDGET(l->data);
        GtkWidget *card = gtk_bin_get_child(GTK_BIN(flow_child));

        int idx = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(card), "video-index"));
        const char *title = si->library->entries[idx].title;

        gboolean visible = (text[0] == '\0') || title_matches(title, text);
        gtk_widget_set_visible(flow_child, visible);
    }
    g_list_free(children);
}
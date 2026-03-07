/*
    Entry-point.

    Creates the GTK window with a GtkStack (home / player).
    When a video thumbnail is clicked, the player starts within
    the same window. When playback ends, it slides back to home.
 */

#include <stdio.h>
#include <gtk/gtk.h>
#include "player.h"
#include "browse_interactor.h"
#include "home_presenter.h"

#define VIDS_DIR "vids"

static Player       g_player = {0};
static HomePresenter g_home;

/* ---- Idle callback: check if a video was selected ---- */

static gboolean check_selection(gpointer data) {
    (void)data;
    const char *path = home_presenter_get_selection(&g_home);
    if (path && !g_player.ctx) {
        player_start(&g_player, &g_home, path);
    }
    return G_SOURCE_CONTINUE;
}

/* ---- Entry point ---- */

int main(int argc, char **argv) {
    gtk_init(&argc, &argv);

    VideoLibrary lib;
    int count = browse_scan_directory(VIDS_DIR, &lib);
    if (count == 0) {
        fprintf(stderr, "No videos found in %s/\n", VIDS_DIR);
        return 1;
    }

    home_presenter_init(&g_home, &lib);

    /* Poll for selection every 100ms */
    g_timeout_add(100, check_selection, NULL);

    gtk_main();

    home_presenter_destroy(&g_home);
    return 0;
}

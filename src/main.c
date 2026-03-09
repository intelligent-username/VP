/*
    Entry-point.

    Creates the GTK window with a GtkStack (home / player).
    When a video thumbnail is clicked, the player starts within
    the same window. When playback ends, it slides back to home.
 */

#include <stdio.h>
#include <string.h>
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
    if (path) {
        char next_path[VE_PATH_MAX];
        int keep_minimized =
            (playback_mode_get_current(&g_home.vp.mode_state) == PLAYBACK_MODE_MINIMIZED);

        strncpy(next_path, path, VE_PATH_MAX - 1);
        next_path[VE_PATH_MAX - 1] = '\0';

        if (g_player.ctx) {
            player_stop(&g_player);
        }

        player_start(&g_player, &g_home, next_path);
        if (keep_minimized && g_player.ctx) {
            vp_set_minimized_mode(&g_home.vp);
            home_presenter_show_home(&g_home);
        }
        home_presenter_clear_selection(&g_home);
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

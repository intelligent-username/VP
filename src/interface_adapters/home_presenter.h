/*
    GTK+3 app window with a GtkStack:
      - "home"   page: video thumbnail grid
      - "player" page: video playback view
 */

#ifndef HOME_PRESENTER_H
#define HOME_PRESENTER_H

#include <gtk/gtk.h>
#include "video_entry.h"
#include "video_presenter.h"

typedef struct {
    GtkWidget      *window;
    GtkWidget      *stack;
    GtkWidget      *home_page;

    GtkWidget *search_entry;     /* search bar at top       */
    GtkWidget *flow_box;         /* reference to thumbnails */

    VideoPresenter  vp;         /* player page lives here   */
    VideoLibrary   *library;    /* borrowed                 */

    char            selected_path[VE_PATH_MAX];
    int             has_selection;
} HomePresenter;

void home_presenter_init(HomePresenter *hp, VideoLibrary *lib);

/* Switch to the home page. */
void home_presenter_show_home(HomePresenter *hp);

/* Switch to the player page. */
void home_presenter_show_player(HomePresenter *hp);

/* Returns the selected path, or NULL. */
const char *home_presenter_get_selection(HomePresenter *hp);
void        home_presenter_clear_selection(HomePresenter *hp);

void home_presenter_destroy(HomePresenter *hp);

#endif

/*
    Player frame rendering and synchronization.
    Handles Cairo drawing and frame display logic.
 */

#ifndef PLAYER_RENDERER_H
#define PLAYER_RENDERER_H

#include "player.h"
#include "app_context.h"
#include "video_presenter.h"

/* Display a frame on the presenter */
void player_renderer_display_frame(Player *p, RGBFrame *f);

/* Attempt to display queued frames with audio sync */
void player_renderer_try_display_frames(Player *p);

/* Check if playback has completed */
int player_renderer_is_playback_over(Player *p);

#endif

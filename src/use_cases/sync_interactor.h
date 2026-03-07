/*
    A/V synchronization decisions.

    Given a video PTS and the current audio playback position,
    decides whether to DISPLAY, DELAY, or DROP a video frame,
    and whether the decoder should skip non-reference frames.
 */

#ifndef SYNC_INTERACTOR_H
#define SYNC_INTERACTOR_H

/* ---------- Input Data (Input Port) ---------- */
typedef struct {
    double video_pts;         /* PTS of video frame to present  */
    double audio_pts;         /* best estimate of audio playback position */
    int    has_audio;         /* 0 = no audio stream            */
    double frame_rate;        /* video fps (for no-audio pacing) */
} SyncInputData;

/* ---------- Output Data (Output Port) ---------- */
typedef enum {
    SYNC_DISPLAY,             /* render frame normally           */
    SYNC_DELAY,               /* sleep first, then render        */
    SYNC_DROP                 /* skip rendering entirely         */
} SyncAction;

typedef enum {
    SKIP_NONE,                /* AVDISCARD_DEFAULT  */
    SKIP_NONREF,              /* AVDISCARD_NONREF   */
    SKIP_NONKEY               /* AVDISCARD_NONKEY   */
} SkipLevel;

typedef struct {
    SyncAction action;
    int        delay_ms;      /* only meaningful when action == SYNC_DELAY */
    SkipLevel  skip_level;    /* hint for decoder frame skipping */
} SyncOutputData;

/* ---------- Interactor ---------- */

/*
 * Evaluate sync between video and audio.
 * Pure function: no side-effects, no framework calls.
 */
SyncOutputData sync_evaluate(const SyncInputData *in);

#endif

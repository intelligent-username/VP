/*
    A/V synchronization decisions.
 */

#include "sync_interactor.h"

/* ---- Thresholds (seconds) ---- */
#define AHEAD_THRESHOLD   0.005   /* video ahead of audio → delay  */
#define BEHIND_THRESHOLD -0.050   /* video behind audio → drop     */
#define SKIP_NONREF_THR  -0.200   /* decoder hint: skip non-ref    */
#define SKIP_NONKEY_THR  -0.500   /* decoder hint: skip to keyframe */
#define MAX_DELAY_MS      200     /* cap sleep to avoid UI freeze  */

static SkipLevel choose_skip_level(double diff) {
    if (diff < SKIP_NONKEY_THR)  return SKIP_NONKEY;
    if (diff < SKIP_NONREF_THR)  return SKIP_NONREF;
    return SKIP_NONE;
}

static int clamp_delay(double diff_sec) {
    int ms = (int)(diff_sec * 1000.0);
    return ms > MAX_DELAY_MS ? MAX_DELAY_MS : ms;
}

static SyncOutputData sync_with_audio(double diff) {
    SyncOutputData out;
    out.skip_level = choose_skip_level(diff);
    out.delay_ms   = 0;

    if (diff > AHEAD_THRESHOLD) {
        out.action   = SYNC_DELAY;
        out.delay_ms = clamp_delay(diff);
    } else if (diff < BEHIND_THRESHOLD) {
        out.action = SYNC_DROP;
    } else {
        out.action = SYNC_DISPLAY;
    }
    return out;
}

static SyncOutputData sync_freerun(double fps) {
    SyncOutputData out;
    out.skip_level = SKIP_NONE;
    out.action     = SYNC_DELAY;
    out.delay_ms   = fps > 0.0 ? (int)(1000.0 / fps) : 33;
    return out;
}

SyncOutputData sync_evaluate(const SyncInputData *in) {
    if (!in->has_audio) {
        return sync_freerun(in->frame_rate);
    }
    double diff = in->video_pts - in->audio_pts;
    return sync_with_audio(diff);
}

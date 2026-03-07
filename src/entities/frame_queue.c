/*
    Thread-safe bounded frame buffer.
 */

#include "frame_queue.h"
#include <stdlib.h>

void fq_init(FrameQueue *q) {
    q->head = q->tail = q->count = 0;
    q->finished = 0;
    q->mutex    = SDL_CreateMutex();
    q->not_full = SDL_CreateCond();
}

void fq_destroy(FrameQueue *q) {
    /* free any remaining frames */
    while (q->count > 0) {
        free(q->frames[q->head].data);
        q->head = (q->head + 1) % FRAME_QUEUE_SIZE;
        q->count--;
    }
    if (q->not_full) SDL_DestroyCond(q->not_full);
    if (q->mutex)    SDL_DestroyMutex(q->mutex);
}

void fq_enqueue(FrameQueue *q, const RGBFrame *f) {
    SDL_LockMutex(q->mutex);
    while (q->count >= FRAME_QUEUE_SIZE && !q->finished) {
        SDL_CondWait(q->not_full, q->mutex);
    }
    if (q->finished) { SDL_UnlockMutex(q->mutex); return; }

    q->frames[q->tail] = *f;
    q->tail = (q->tail + 1) % FRAME_QUEUE_SIZE;
    q->count++;
    SDL_UnlockMutex(q->mutex);
}

RGBFrame *fq_peek(FrameQueue *q) {
    SDL_LockMutex(q->mutex);
    RGBFrame *r = (q->count > 0) ? &q->frames[q->head] : NULL;
    SDL_UnlockMutex(q->mutex);
    return r;
}

int fq_dequeue(FrameQueue *q, RGBFrame *out) {
    SDL_LockMutex(q->mutex);
    if (q->count == 0) { SDL_UnlockMutex(q->mutex); return 0; }

    *out = q->frames[q->head];
    q->head = (q->head + 1) % FRAME_QUEUE_SIZE;
    q->count--;
    SDL_CondSignal(q->not_full);
    SDL_UnlockMutex(q->mutex);
    return 1;
}

void fq_signal_finished(FrameQueue *q) {
    SDL_LockMutex(q->mutex);
    q->finished = 1;
    SDL_CondBroadcast(q->not_full);
    SDL_UnlockMutex(q->mutex);
}

int fq_is_empty(FrameQueue *q) {
    SDL_LockMutex(q->mutex);
    int empty = (q->count == 0);
    SDL_UnlockMutex(q->mutex);
    return empty;
}

/*
    Thread-safe bounded frame buffer.
    Sits between the video decode thread and the GTK render timer.
 */

#ifndef FRAME_QUEUE_H
#define FRAME_QUEUE_H

#include <stdint.h>
#include <SDL2/SDL.h>

#define FRAME_QUEUE_SIZE 4   /* small, prevents huge memory use */

typedef struct {
    uint8_t *data;           /* BGRA pixel data (malloc'd) */
    int      width;
    int      height;
    int      stride;         /* bytes per row = width * 4   */
    double   pts;            /* presentation timestamp (s)  */
} RGBFrame;

typedef struct {
    RGBFrame  frames[FRAME_QUEUE_SIZE];
    int       head, tail, count;
    SDL_mutex *mutex;
    SDL_cond  *not_full;     /* signalled when a slot opens */
    int       finished;      /* 1 when producer is done     */
} FrameQueue;

void  fq_init(FrameQueue *q);
void  fq_destroy(FrameQueue *q);

/* Blocking enqueue: waits if full. Skips if finished. */
void  fq_enqueue(FrameQueue *q, const RGBFrame *f);

/* Non-blocking peek: returns pointer or NULL. */
RGBFrame *fq_peek(FrameQueue *q);

/* Non-blocking dequeue: returns 1 if got a frame, 0 if empty. */
int   fq_dequeue(FrameQueue *q, RGBFrame *out);

/* Signal the producer to stop blocking. */
void  fq_signal_finished(FrameQueue *q);

int   fq_is_empty(FrameQueue *q);

#endif

/*
    SDL bootstrap (audio only).
 */

#include "sdl_init.h"
#include <SDL2/SDL.h>
#include <stdio.h>

int sdl_bootstrap(void) {
    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return -1;
    }
    return 0;
}

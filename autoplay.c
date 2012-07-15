#include "autoplay.h"
#include "stdlib.h"

#include <SDL/SDL.h>

#define TICKCOUNT 1000

int timeElapsed = 0;
int lastCall = 0;

void autoplay_init()
{
    lastCall = SDL_GetTicks();
    timeElapsed = 0;
}

int autoplay_update(SDL_keysym *key)
{

    // timer
    int newTime = SDL_GetTicks();
    timeElapsed += newTime - lastCall;
    lastCall = newTime;
    if (timeElapsed < TICKCOUNT)
        return 0;
    timeElapsed = 0;

    // generate random keypress
    switch (rand() % 4) {
    case 0: key->sym = SDLK_LEFT; break;
    case 1: key->sym = SDLK_RIGHT; break;
    case 2: key->sym = SDLK_UP; break;
    case 3: key->sym = SDLK_DOWN; break;
    }
    return 1;
}

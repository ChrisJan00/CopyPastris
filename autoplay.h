#ifndef _AUTOPLAY_H_
#define _AUTOPLAY_H_

#include "SDL/SDL.h"

// it just simulates a player
// AI model used: monkey banging a keyboard randomly
void autoplay_init();
int autoplay_update(SDL_keysym *key);

#endif

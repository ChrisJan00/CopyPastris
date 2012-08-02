// SDL tetris
// Copyright (c) 2009 Dominik Zaczkowski
//
// Copypastris
// Copyright (c) 2012 Christiaan Janssen
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

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

#ifndef _TEXT_H_
#define _TEXT_H_

#include <SDL/SDL.h>

void init_font(const char *);
void sf_puts(SDL_Surface *, SDL_Rect *, const char *);
SDL_Rect sf_gets(SDL_Surface *, SDL_Rect *, char * const, int);
void sf_printf(SDL_Surface *, SDL_Rect *, const char *, ...);

#endif

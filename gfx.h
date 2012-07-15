#ifndef _GFX_H_
#define _GFX_H_

#include <SDL/SDL.h>

SDL_Surface *create_surface_wrap(int, int, int, const char *);
#define create_surface(w, h)	create_surface_wrap(w, h, __LINE__, __FILE__)
void fill_gradient(SDL_Surface *);
void draw_border(SDL_Surface *, SDL_Rect, int, Uint32);

#endif

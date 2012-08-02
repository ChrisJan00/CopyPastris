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

#include <stdio.h>
#include <stdlib.h>

#include <SDL/SDL.h>

typedef Uint8 Pixel[3];

static Uint32 interpolate_two_colors(SDL_PixelFormat *, Pixel, Pixel, int, int);

SDL_Surface *
create_surface_wrap(int width, int height, int line, const char *file)
{
	extern SDL_Surface *screen;
	extern Uint32 video_flags;
	SDL_Surface *ret;
	SDL_PixelFormat *fmt = screen->format;

    ret = SDL_CreateRGBSurface(video_flags, width, height,
	    fmt->BitsPerPixel, fmt->Rmask, fmt->Gmask, fmt->Bmask,
	    fmt->Amask);

	if (ret == NULL) {
		fprintf(stderr, "%s: %d: %s\n", file, line,
		    SDL_GetError());

		exit(EXIT_FAILURE);
	}
	       
	return ret;
}

/*
 * funkcja tworzy gradient na powierzchni *sf
 */
void
fill_gradient(SDL_Surface *sf)
{
	SDL_Rect r;
    Pixel A = {0x41, 0x3a, 0x02};
    Pixel B = {0xcf, 0xcf, 0x04};

	const int steps = sf->w;
	int i;

	r.y = 0;
	r.h = sf->h;
	r.w = sf->w / steps;
	r.x = 0;
	for (i = 0; i <= steps; ++i) {
		Uint32 c = interpolate_two_colors(sf->format, A, B, i, steps);

		SDL_FillRect(sf, &r, c);
		r.x = i * r.w;
	}
	
}
		
/*
 * Funkcja rysuje ramke wokol prostokatu "r" o kolorze "color" i grubosci
 * "th" na powierzchni "sf"
 */
void
draw_border(SDL_Surface *sf, SDL_Rect r, int th, Uint32 color)
{
	SDL_Rect line;

	/* gora */
	line.x = r.x - th;
	line.y = r.y - th;
	line.w = r.w + 2 * th;
	line.h = th;
	SDL_FillRect(sf, &line, color);
	/* dol */
	line.y = r.y + r.h;
	SDL_FillRect(sf, &line, color);
	/* lewa */
	line.y = r.y;
	line.w = th;
	line.h = r.h;
	SDL_FillRect(sf, &line, color);
	/* prawa */
	line.x = r.x + r.w;
	SDL_FillRect(sf, &line, color);
}

static Uint32
interpolate_two_colors(SDL_PixelFormat *fmt, Pixel A, Pixel B, int step,
    int max_steps)
{
	Pixel V;
	int i;
	
	for (i = 0; i < sizeof(Pixel); ++i)
		V[i] = (B[i] - A[i]) * step / max_steps + A[i];

	return SDL_MapRGB(fmt, V[0], V[1], V[2]);
}

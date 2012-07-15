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
	Pixel A = {0, 0, 0};
	Pixel B = {255, 0, 255};
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

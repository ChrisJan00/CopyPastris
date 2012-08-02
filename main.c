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

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>

#include <SDL/SDL.h>

#include "common.h"
#include "game.h"
#include "gfx.h"
#include "mainloop.h"
#include "tetris.h"
#include "text.h"
#include "xerror.h"

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480
#define DEPTH		16

#define ICON_W		32
#define ICON_H		32

//#ifndef DATA
//#error DATA directory not defined!
//#endif

#define CLEAR_BIT(byte, bit)	(byte &= ~(1 << bit))

Uint32 video_flags = SDL_SWSURFACE;
SDL_Surface *screen;
SDL_Surface *bground;

static Uint8 *make_icon_mask(SDL_Surface *);

int
main(int argc, char *argv[])
{
	struct position p;
	struct game *g;
	SDL_Surface *icon;

	/* iniclalizacja SDL */
	if (SDL_Init(SDL_INIT_VIDEO) == -1)
		ERROR("SDL_Init: %s", SDL_GetError());

	atexit(SDL_Quit);
    icon = SDL_LoadBMP("tetris.bmp");
	if (icon)
		SDL_WM_SetIcon(icon, make_icon_mask(icon));
	SDL_WM_SetCaption(PACKAGE_NAME, NULL);

	screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, DEPTH,
	    video_flags);
	if (screen == NULL)
		ERROR("SDL_SetVideoMode: %s", SDL_GetError());

	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, 
	    SDL_DEFAULT_REPEAT_INTERVAL);

	/* set random seed */
	srand(time(NULL));

	/* inicjalizacja fontow */
#ifdef MAC_OS_X_VERSION_10_6
    init_font("copypastris.app/Contents/Resources/FreeMonoBold.ttf");
#else
    init_font("FreeMonoBold.ttf");
#endif    
    
	bground = create_surface(SCREEN_WIDTH, SCREEN_HEIGHT);
	fill_gradient(bground);
	/* TODO: zrobic to ansi C kompatybilne */
	sf_puts(bground, &(SDL_Rect){.x = 20, .y = SCREEN_HEIGHT - 20},
        "ESC - quit, S - new game, F - fullscreen");

	p = compute_pos(SCREEN_WIDTH, SCREEN_HEIGHT);
	draw_matrix_border(&p);
	create_color_blocks(p.size);

	SDL_Flip(screen);

	g = init_game(redraw, level_up, gameover, draw_tetramino, 
	    tetramino_preview, &p, 1);

    drawInstructions(g);
	main_loop(g);

    free_blocks();

	return 0;
}

static Uint8 *
make_icon_mask(SDL_Surface *icon)
{
	static Uint8 mask[ICON_W * ICON_H];
	Uint8 *byte = mask;
	int i;

	if (icon->w > ICON_W || icon->h > ICON_H)
		return NULL;

	/* set all bits 1 */
	for (i = 0; i < ICON_H * ICON_W; ++i)
		mask[i] = UCHAR_MAX;
	/*
	 * HACK: zakladam ze scanlinia jest takiej samej dlugosci jak icon->w
	 */
	SDL_LockSurface(icon);
	for (i = 0; i < icon->w * icon->h; ++byte) {
		int b;
		for (b = 7; b >= 0 ; --b, ++i)
			switch (icon->format->BitsPerPixel) {
			case 8:
				if (((Uint8 *)icon->pixels)[i] == UCHAR_MAX)
					CLEAR_BIT(*byte, b);
				break;
			case 16:
				if (((Uint16 *)icon->pixels)[i] == USHRT_MAX)
					CLEAR_BIT(*byte, b);
				break;
			case 24: {
				typedef Uint8 (*Pixel)[3];
				Pixel p = &((Pixel)icon->pixels)[i];

				if ((*p)[0] == UCHAR_MAX 
				    && (*p)[1] == UCHAR_MAX
				    && (*p)[2] == UCHAR_MAX)
					CLEAR_BIT(*byte, b);

				break;
			}
			default:
				;
			}
	}
	SDL_UnlockSurface(icon);

	return mask;
}

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

#ifndef _GAME_H_
#define _GAME_H_

#include "tetris.h"

#define BORDER		3
#define COLOR_DIFF	20
#define MINIMAL_SQUARE_SIZE	10
#define MAX_DARK_COLOR	90
/* niech podglad ma rozmiary 9x4, tak dla pewnosci ze zawsze wszystko
 * sie ladnie narysuje */
#define PREVIEW_W	9
#define PREVIEW_H	4

struct position {
	int size;			/* rozmiar klocka */
	int x, y;			/* pozycja gornego lewego rogu */
	int x2, y2;			/* pozycja tekstu i podgladu */
};

void level_up(struct game *);
void gameover(struct game *);
void redraw(struct game *);
void draw_tetramino(struct game *, const int *, int, int);
void tetramino_preview(struct game *, const int *, int);
void create_color_blocks(int);
struct position compute_pos(int, int);
void draw_matrix_border(const struct position *);
void draw_block(struct game *g, int x, int y, int color, bool valid);
void draw_block_mark(struct game *g, int x, int y, int color);
void free_blocks();

#endif

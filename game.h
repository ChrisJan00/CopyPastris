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

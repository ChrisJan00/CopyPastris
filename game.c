#include <string.h>

#include <SDL/SDL.h>

#include "highscores.h"
#include "game.h"
#include "gfx.h"
#include "mainloop.h"
#include "text.h"

#define MSG_DELAY	1000

extern SDL_Surface *screen;
extern SDL_Surface *bground;
/* wzorzec koloru kafla odpowiadajacy enumeratorowi "enum color" z tetris.h */
static const Uint8 block_color[COLOR_COUNT][3] =
{{230, 30, 30}, {120, 120, 120}, {30, 230, 230},
 {230, 230, 30}, {230, 30, 240}, {30, 30, 230},
 {30, 230, 30}};
/* tablica klockow z ktorych beda rysowane tetramino, kolor elementu tablicy
 * odpowiada enumeratorowi "enum color" */
static SDL_Surface *block[COLOR_COUNT];
static Uint8 sub0(int, int);
static Uint8 add0(int, int);
static SDL_Surface *selectOverlay;
static SDL_Surface *markOverlay;
static SDL_Surface *previewOverlay;
static SDL_Surface *invalidOverlay;

void
level_up(struct game *g)
{
	SDL_Rect r;
	const struct position *p = g->frontend;

	++g->level;

	r.x = p->x + 3 * p->size;
	r.y = p->y + 7 * p->size;
	sf_puts(screen, &r, "Level Up!");
	SDL_UpdateRect(screen, r.x, r.y, r.w, r.h);
	SDL_Delay(MSG_DELAY);
	SDL_BlitSurface(bground, &r, screen, &r);
	SDL_UpdateRect(screen, r.x, r.y, r.w, r.h);
	
	/* zwiekszam szybkosc spadania tetramino */
	if (g->fall_time > FALL_BY_LEVEL)
		g->fall_time -= FALL_BY_LEVEL;
	else 
		g->fall_time /= 2;
}

void
gameover(struct game *g)
{
	SDL_Rect r;
	struct position *p = g->frontend;
	int pph;			/* points per hour */

#if 0
	if (g->running == false)
		return;
#endif
	pph = g->points * 3600.f / (time(NULL) - g->game_time);

	if (high_score(g->lines_cleared, g->points, pph)) {
		char buf[20];

		r.x = p->x2;
		r.y = p->y2 + p->size * PREVIEW_H + 60;
		sf_puts(screen, &r, "Highscore, congratulations!\n"
		    "Please enter your name:");
		SDL_UpdateRects(screen, 1, &r);

		r.y += r.h;

		sf_gets(bground, &r, buf, sizeof(buf));
		if (*buf == '\0')
			strcpy(buf, "Anonymous");
		write_scores(buf, g->lines_cleared, g->points, pph);
	}

#if 0	
	g->running = false;
#endif
	restart_game(g);
	draw_hscores(g);
}

/* funkcja rysuje caly matrix */
void
redraw(struct game *g)
{
	SDL_Rect r;
	const struct position *p = g->frontend;
	int i, j;

	r.w = r.h = p->size;
	for (i = 0; i < MATRIX_HEIGHT; ++i) 
		for (j = 0; j < MATRIX_WIDTH; ++j) {
			const struct square *sq = &g->m[i * MATRIX_WIDTH];
			    
			r.x = p->x + p->size * j;
			r.y = p->y + p->size * i;

            if (sq[j].visible == true) {
				SDL_BlitSurface(block[sq[j].color], NULL,
				    screen, &r);
                if (sq[j].selected)
                    SDL_BlitSurface(selectOverlay, NULL, screen, &r);
                else if (sq[j].marked)
                    SDL_BlitSurface(markOverlay, NULL, screen, &r);
            }
			else 
				SDL_BlitSurface(bground, &r, screen, &r);
		}

	SDL_Flip(screen);
}

/* funkcja rysuje tetramino na matriksie, w tablicy blocks jest bg
 * kafli do odrysowania na kolor tla i zaraz po nich tc kafli do narysowania 
 * na kolor aktualnego tetramino.
 */
void
draw_tetramino(struct game *g, const int *blocks, int bg, int tc)
{
	SDL_Rect update[TETRAMINO_MAX * 2];
	SDL_Rect r;
	const struct position *p = g->frontend;
	/* pozycja tetramino na matriksie jako talibcy 1d */
	const int pos = g->x + g->y * MATRIX_WIDTH;
	int i;

	r.w = r.h = p->size;
	for (i = 0; i < bg + tc; ++i) {
		int pb, x, y;	      /* pozycja bloku na matriksie */

		/* wyliczenie pozycji bloku we wspolrzednych (x, y) */
		pb = pos + blocks[i];
		x = pb % MATRIX_WIDTH;
		y = pb / MATRIX_WIDTH;

		r.x = p->x + x * p->size;
		r.y = p->y + y * p->size;

        if (i < tc) {
            if (g->visible) {
                SDL_BlitSurface(block[g->cur_tetramino], NULL, screen, &r);
                if (g->selected)
                    SDL_BlitSurface(selectOverlay, NULL, screen, &r);
                else if (g->marked)
                    SDL_BlitSurface(markOverlay, NULL, screen, &r);
            } else
                if (!draw_preview_in_pos(g, x, y))
                    SDL_BlitSurface(bground, &r, screen, &r);
        }
		else
            if (!draw_preview_in_pos(g, x, y))
                SDL_BlitSurface(bground, &r, screen, &r);

		update[i] = r;
	}

	SDL_UpdateRects(screen, i, update);
}

void draw_block(struct game *g, int x, int y, int color, bool valid)
{
    SDL_Rect r;
    const struct position *p = g->frontend;
    r.w = r.h = p->size;
    r.x = p->x + x * p->size;
    r.y = p->y + y * p->size;
    SDL_BlitSurface(block[color], NULL, screen, &r);
    if (valid)
        SDL_BlitSurface(previewOverlay, NULL, screen, &r);
    else
        SDL_BlitSurface(invalidOverlay, NULL, screen, &r);
//    SDL_UpdateRects(screen, 1, &r);
}

void draw_block_mark(struct game *g, int x, int y, int color)
{
    SDL_Rect r;
    const struct position *p = g->frontend;
    r.w = r.h = p->size;
    r.x = p->x + x * p->size;
    r.y = p->y + y * p->size;
    SDL_BlitSurface(block[color], NULL, screen, &r);
    SDL_BlitSurface(markOverlay, NULL, screen, &r);
//    SDL_UpdateRects(screen, 1, &r);
}

void call_updaterects(struct game *g, int x, int y, int w, int h)
{
    if (w < 1 || h < 1)
        return;

    const struct position *p = g->frontend;
    SDL_Rect r;
    r.x = p->x + x * p->size;
    r.y = p->y + y * p->size;
    r.w = w * p->size;
    r.h = h * p->size;
    SDL_UpdateRect(screen, r.x, r.y, r.w, r.h);
}

void draw_field(struct game *g, int x, int y)
{
    SDL_Rect r;
    const struct position *p = g->frontend;
    r.w = r.h = p->size;
    r.x = p->x + x * p->size;
    r.y = p->y + y * p->size;

    int pos = x + y * MATRIX_WIDTH;

    if (g->m[pos].visible) {
        SDL_BlitSurface(block[g->m[pos].color], NULL, screen, &r);
        if (g->m[pos].selected)
            SDL_BlitSurface(selectOverlay, NULL, screen, &r);
        else if (g->m[pos].marked)
            SDL_BlitSurface(markOverlay, NULL, screen, &r);
    }
    else
        SDL_BlitSurface(bground, &r, screen, &r);
}

/*
 * funkcja wyswietla tetramino jakie poleci w nastepnej kolejnosci,
 * przy okazji jak jest nowe tetramino (bo wtedy jest odpalana), to mozna
 * wypisywac punkty i linie.
 */
void
tetramino_preview(struct game *g, const int *tmino, int size)
{
	static SDL_Rect r[2];
	struct position *p = g->frontend;
	int i;
	
	r[0].x = p->x2;
	r[0].y = p->y2;
	r[0].w = p->size * PREVIEW_W;
	r[0].h = p->size * PREVIEW_H;
	/* TODO: zrobic ten preview szybszy? */
	SDL_BlitSurface(bground, &r[0], screen, &r[0]);

	/* teraz petla po kaflach tetramino */
	for (i = 0; i < size; ++i) {
		/* TODO: zrobic to ansi c kompatybilne */
		SDL_Rect r = {p->x2, p->y2, p->size, p->size};
		const int pos = PREVIEW_W / 2 + tmino[i];
		const int x = pos % MATRIX_WIDTH;
		const int y = pos / MATRIX_WIDTH;
		
		r.x += p->size * x;
		r.y += p->size * y;

		SDL_BlitSurface(block[g->next_tetramino], NULL, screen, &r);
	}
	/* w tym miejscu juz stary r */

	/* TODO: do oddzielnej funkcji z tym */
	SDL_BlitSurface(bground, &r[1], screen, &r[1]);
	r[1].x = r[0].x;
	r[1].y = r[0].y + r[0].h + 5;
	sf_printf(screen, &r[1], "Points: %4d\nLines : %4d\nLevel : %4d", 
	    g->points, g->lines_cleared, g->level);

	SDL_UpdateRects(screen, sizeof(r) / sizeof(r[0]), r);
}

/* funkcja tworzy bloki tworzace tetramino, zwalnia pamiec juz istniejacych, 
 * jesli takie sa */
void
create_color_blocks(int size)
{
	enum color i;

	/* zwalniam pamiec ze starych blokow jezeli istnieja */
	for (i = 0; i < COLOR_COUNT; ++i)
		if (block[i] != NULL)
			SDL_FreeSurface(block[i]);

	/* tworze nowe klocki */
	for (i = 0; i < COLOR_COUNT; ++i) {
		SDL_Rect r;
		Uint32 color;
		int th = size * 0.1f;
		block[i] = create_surface(size, size);

		/* srodek */
		color = SDL_MapRGB(block[i]->format, block_color[i][0],
		    block_color[i][1], block_color[i][2]);
		SDL_FillRect(block[i], NULL, color);

		/* gora i lewy bok */
		color = SDL_MapRGB(block[i]->format, 
		    add0(block_color[i][0], COLOR_DIFF),
		    add0(block_color[i][1], COLOR_DIFF),
		    add0(block_color[i][2], COLOR_DIFF));

		r.x = 0; r.y = 0; r.w = block[i]->w; r.h = th;
		SDL_FillRect(block[i], &r, color);
		r.w = th; r.h = block[i]->h - th;
		SDL_FillRect(block[i], &r, color);

		/* dol i prawy bok */
		color = SDL_MapRGB(block[i]->format, 
		    sub0(block_color[i][0], COLOR_DIFF),
		    sub0(block_color[i][1], COLOR_DIFF),
		    sub0(block_color[i][2], COLOR_DIFF));

		r.y = block[i]->h - th; r.w = block[i]->w; r.h = th;
		SDL_FillRect(block[i], &r, color);
		r.x = block[i]->w - th; r.y = r.w = th; r.h = block[i]->h - th;
		SDL_FillRect(block[i], &r, color);
	}

    // overlays
    Uint32 color;
    Uint8 alpha = 0x80;

    selectOverlay = create_surface(size, size);
    color = SDL_MapRGB(block[0]->format, 0, 0, 0xff);
    SDL_FillRect(selectOverlay, NULL, color);
    SDL_SetAlpha(selectOverlay, SDL_SRCALPHA, alpha);

    markOverlay = create_surface(size, size);
    color = SDL_MapRGB(block[0]->format, 0, 0, 0);
    SDL_FillRect(markOverlay, NULL, color);
    SDL_SetAlpha(markOverlay, SDL_SRCALPHA, alpha);

    previewOverlay = create_surface(size, size);
    color = SDL_MapRGB(block[0]->format, 0, 0xff, 0);
    SDL_FillRect(previewOverlay, NULL, color);
    SDL_SetAlpha(previewOverlay, SDL_SRCALPHA, alpha);

    invalidOverlay = create_surface(size, size);
    color = SDL_MapRGB(block[0]->format, 0xff, 0, 0);
    SDL_FillRect(invalidOverlay, NULL, color);
    SDL_SetAlpha(invalidOverlay, SDL_SRCALPHA, alpha);
}

void free_blocks()
{
    int i;
    for (i = 0; i < COLOR_COUNT; ++i)
        if (block[i] != NULL)
            SDL_FreeSurface(block[i]);
    SDL_FreeSurface(selectOverlay);
    SDL_FreeSurface(markOverlay);
    SDL_FreeSurface(previewOverlay);
    SDL_FreeSurface(invalidOverlay);
}

/* funkcja oblicza gdzie ma sie znajdowac matrix i zwraca strukture z tymi 
   danymi */
struct position
compute_pos(int screen_width, int screen_height)
{
	struct position p;
	
	/* w zaleznosci co mniejsze wysokosc czy szerokosc okna licze rozmiar
	 * klocka */
	if (screen_height < screen_width) 
		p.size = screen_height / (MATRIX_HEIGHT + 4);
	else
		p.size = screen_width / (MATRIX_WIDTH + 4);
	
	if (p.size < MINIMAL_SQUARE_SIZE) {
		fprintf(stderr, "Block size is %dpx, but can't be less than"
		    " %dpx.\nPlease resize your window.\nExiting.\n", p.size,
		    MINIMAL_SQUARE_SIZE);
		
		exit(EXIT_FAILURE);
	}

	/* niech matrix bedzie po lewej */
	p.y = p.size * 2;
	p.x = p.size * 2;
	/* TODO: co jak sie podglad juz nie zmiesci na ekranie? */
	/* pozycja podgladu tetramino i licznikow punktow itp. */
	p.x2 = p.y + p.size * (MATRIX_WIDTH + 2);
	p.y2 = p.y;
	
	
	return p;
}

/* funkcja rysuje ramke wokol matriksu */
void
draw_matrix_border(const struct position *p)
{
	SDL_Rect r;
	Uint32 c;

	/* TOOD: konfigurowalny kolor zrobic */
	c = SDL_MapRGB(screen->format, 123, 11, 53);
	r.x = p->x;
	r.y = p->y;
	r.w = p->size * MATRIX_WIDTH;
	r.h = p->size * MATRIX_HEIGHT;
	draw_border(bground, r, BORDER, c);
	/* Teraz ramka wokol podgladu klocka */
	r.x = p->x2;
	r.w = p->size * PREVIEW_W;
	r.y = p->y2;
	r.h = p->size * PREVIEW_H;
	draw_border(bground, r, BORDER, c);

	SDL_BlitSurface(bground, NULL, screen, NULL);
	SDL_Flip(screen);
}

static Uint8
sub0(int a, int b)
{
	return a - b < 0 ? 0 : a - b;
}

static Uint8
add0(int a, int b)
{
	return a + b >= 255 ? 255 : a + b;
}

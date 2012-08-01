/*
 * Idea jest taka: tetris to gra dosc wolna - dlatego do sledzenia "kolizji"
 * uzyje macierzy "kwadratow" ktora bedzie rownoczesnie plansza gry,
 * wszystko bedzie sie odbywac na ww. macierzy, a program bedzie to rysowal
 * na eranie.
 */
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "random.h"
#include "tetris.h"
#include "xmalloc.h"

#include "autoplay.h"
#include "mouseinput.h"

/* Wiersze */
#define W0	0
#define W1	MATRIX_WIDTH
#define W2	(2 * MATRIX_WIDTH)	    
#define W3	(3 * MATRIX_WIDTH)
/* Kolumny */
#define L	-1
#define R	1

/* struktura pomocnicza do zwracania pozycji kafli na matriksie, ktore maja
 * byc narysowane od nowa, na kolor tla i na kolor tetramino */
struct update {
	int square[TETRAMINO_MAX * 2];
	int background;
	int color;
};

static const struct tetramino tetramino[] = {
	/* TETRA_l */
	{{{L, W0, R, 2 * R},
	  {W0, W1, W2, W3}, }, 2, 4},
	/* TETRA_T */
	{{{L, W0, R, W1},
	  {W0, W1, W1 + R, W2},
	  {W0, W1, W1 + L, W1 + R},
	  {W0, W1, W2, W1 + L}}, 4, 4},
	/* TETRA_O */
	{{{W0, R, W1, W1 + R}, }, 1, 4},
	/* TETRA_L */
	{{{W0, W1, W2, W2 + R},
	  {W1 + L, W1, W1 + R, R},
	  {W0, R, W1 + R, W2 + R},
	  {L, W1 + L, W0, R}}, 4, 4},
	/* TETRA_J */
	{{{W0, W1, W2, W2 + L},
	  {R, W0, L, W1 + R},
	  {W0, R, W1, W2},
	  {L, W1 + L, W1, W1 + R}}, 4, 4},
	/* TETRA_S */
	{{{W0, R, W1, W1 + L},
	  {W0, W1, W1 + R, W2 + R}}, 2, 4},
	/* TETRA_Z */
	{{{L, W0, W1, W1 + R},
	  {W0, W1, W1 + L, W2 + L}}, 2, 4}
};

#undef W
#undef W1
#undef W2
#undef W3
#undef L
#undef R
	

static int clear_lines(struct game *);
//static bool check_bounds(const int *, int, int, int);
static struct update update_tetramino(const int *, const int *, int, int);
//static bool check_squares(const struct game *, const int *, int, int, int);
static void put_tetramino(struct game *, const int *, int);
static int round_to_ten(int);

const struct tetramino *get_tetramino(int selection)
{
    return &tetramino[selection];
}

/* funkcja inicjujaca strukture "struct game" */
struct game *
init_game(void (*redraw)(struct game *), void (*levelup)(struct game *),
    void (*gameover)(struct game *),
    void (*move)(struct game *, const int *, int, int),
    void (*pview)(struct game *, const int *, int), 
    void *frontend, int level)
{
	struct game *g;
	int i;

	g = xmalloc(sizeof(*g));

	g->points = g->lines_cleared = 0;
	g->level = level;
	g->redraw = redraw;
	g->levelup = levelup;
	g->gameover = gameover;
	g->moved_tetramino = move;
	g->preview = pview;
	g->frontend = frontend;
	g->running = true;
	g->fall_time = FALL_TIME;

	time(&g->game_time);

	/* rotacja pierwszego klocka */
	g->next_tetramino = get_random(0, sizeof tetramino 
	    / sizeof tetramino[0]);
	g->next_rotation = 0;

	/* wyczyszczenie matriksu */
	for (i = 0; i < MATRIX_SIZE; ++i)
		g->m[i].visible = false;

    autoplay_init();
    clean_buffers(g);

	return g;
}

void
restart_game(struct game *g)
{
	int i;

	for (i = 0; i < sizeof(g->m) / sizeof(g->m[0]); ++i)
		g->m[i].visible = false;

	g->fall_time = FALL_TIME;
	g->level = 1;
	g->lines_cleared = g->points = 0;
	g->running = true;
	g->redraw(g);
	new_tetramino(g);

    autoplay_init();
    clean_buffers(g);
}

void
rotate_tetramino(struct game *g, bool clockwise)
{
	const struct tetramino *t = &tetramino[g->cur_tetramino];
	int r;				/* nowa rotacja */
	int d = clockwise ? 1 : -1;	/* kierunek rotacji */
	struct update u;
	
	/* znajduje nowa rotacje klocka */
	switch (g->cur_tetramino) {
	case TETRA_O:
		return;
	case TETRA_Z:
	case TETRA_S:
	case TETRA_l:
		r = !g->rotation;
		break;
	case TETRA_T:
	case TETRA_L:
	case TETRA_J:
		r = g->rotation + d;
		if (r < 0)
			r = 3;
		else if (r >= 4)
			r = 0;
		break;
	}

	/* sprawdzam czy rotacja jest mozliwa */
	if (!check_bounds(t->rotation[r], t->size, g->x, g->y))
		return;
	/* sprawdzenie czy tetramino nie wbije sie w juz ulozone klocki */
	if (!check_squares(g, t->rotation[r], t->size, g->x, g->y))
		return;
	
	u = update_tetramino(t->rotation[g->rotation], t->rotation[r], 
	    t->size, t->size);
    // extra
    recheckMouse(g);
	g->moved_tetramino(g, u.square, u.background, u.color);
	g->rotation = r;
}

/* przesuniecie tetramino w lewo badz w prawo */
void
move_tetramino(struct game *g, enum direction d)
{
	int new_t[TETRAMINO_MAX];	/* nowa pozycja tetramino */
 	struct update u;
	/* aktualne tetramino */
	const struct tetramino *t = &tetramino[g->cur_tetramino];
	int t_pos, x, y;
	int i;

	/* sprawdzam czy nie kaze tetramino wyjsc za matrix */
	if (g->x == 0 && d == LEFT || g->x == MATRIX_WIDTH - 1 && d == RIGHT)
		return;

	t_pos = g->x + g->y * MATRIX_WIDTH + d;
	x = t_pos % MATRIX_WIDTH;
	y = t_pos / MATRIX_WIDTH;

	/* sprawdzam czy moge przesunac */
	if (!check_bounds(t->rotation[g->rotation], t->size, x, y) ||
	    !check_squares(g, t->rotation[g->rotation], t->size, x, y)) {
		if (d == DOWN) {
			put_tetramino(g, t->rotation[g->rotation], t->size);
			new_tetramino(g);
		}
		return;
	}

	/* tworze "nowe" tetramino przez dodanie do "starego" kierunku */
	for (i = 0; i < t->size; ++i)
		new_t[i] = t->rotation[g->rotation][i] + d;

	u = update_tetramino(t->rotation[g->rotation], new_t, t->size, t->size);
    // extra
    recheckMouse(g);
	g->moved_tetramino(g, u.square, u.background, u.color);
	g->x = x;
	g->y = y;
}

void
fast_forward(struct game *g)
{
	int t[TETRAMINO_MAX];		/* bloki tetramino */
	struct update u;
	const int *tmino = tetramino[g->cur_tetramino].rotation[g->rotation];
	const int size = tetramino[g->cur_tetramino].size;
	int y = g->y;
	int i;
	
	/* znajduje ostatni y na ktorym tetramino z niczym nie koliduje */
	while (check_bounds(tmino, size, g->x, y + 1)
	    && check_squares(g, tmino, size, g->x, y + 1))
		++y;

	/* nowe tetramino dla update_tetramino() musi miec odleglosci od
	 * (g->x, g->y) tak jak stare tetramino */
	for (i = 0; i < size; ++i)
		t[i] = tmino[i] + (y - g->y) * MATRIX_WIDTH;

	u = update_tetramino(tmino, t, size, size);
    recheckMouse(g);
	g->moved_tetramino(g, u.square, u.background, u.color);

	g->y = y;
	put_tetramino(g, tmino, size);
	new_tetramino(g);
}
	
void
new_tetramino(struct game *g)
{
	const struct tetramino *t;
	g->x = MATRIX_WIDTH / 2;
	g->y = 0;

	g->cur_tetramino = g->next_tetramino;
	g->rotation = g->next_rotation;
	g->next_tetramino = get_random(0, sizeof tetramino 
	    / sizeof tetramino[0]);
	t = &tetramino[g->next_tetramino];
	g->next_rotation = get_random(0, t->rotations);

    g->selected = false;
    g->marked = false;
    g->visible = true;

	if (g->preview)
		g->preview(g, t->rotation[g->next_rotation], t->size);

	t = &tetramino[g->cur_tetramino];

	/* sprawdzam czy nowe tetramino moze sie pojawic, jak nie to gameover */
	if (!check_squares(g, t->rotation[g->rotation], t->size, g->x, g->y)) {
		if (g->gameover)
			g->gameover(g);
		    
		return;
	}

    g->moved_tetramino(g, t->rotation[g->rotation], 0, t->size);
}

void
redraw_all(struct game *g)
{
	const struct tetramino *t;
	
	if (g->preview) {
		t = &tetramino[g->next_tetramino];
		g->preview(g, t->rotation[g->next_rotation], t->size);
	}

	g->redraw(g);

	t = &tetramino[g->cur_tetramino];
	g->moved_tetramino(g, t->rotation[g->rotation], 0, t->size);
}

/* funkcja sprawdza czy dane tetramino miesci sie na planszy,
 * -- tmino[len] - tablica "wskazujaca" na klocki tworzace tetramino.
 * -- x, y - miejsce gdzie jest tetramino.
 * Zwraca prawde jesli terramino miesci sie na planszy.
 */
bool
check_bounds(const int *tmino, int len, int x, int y)
{
	int i;
	/* pozycja tetramino */
	const int t_pos = x + y * MATRIX_WIDTH;

	/* wpierw sprawdzam czy sama pozycja znajduje sie na planszy */
	if (x < 0 || x > MATRIX_WIDTH || y < 0 || y > MATRIX_HEIGHT)
		return false;

	for (i = 0; i < len; ++i) {
		/* pozycja bloku */
		const int b_pos = tmino[i] + t_pos;
		/* sprawdzam ile wysokosci "miesci" sie w klocku, odejmuje je
		 * i w ten sposob otrzymuje kierunek klocka -- ile w prawo,
		 * ile w lewo, dodaje do pozycji w ktorej ma byc tetramino
		 * i juz moge sprawdzac czy nic nie wyskakuje poza matrix */
		int x_pos = tmino[i] - round_to_ten(tmino[i]) + x;
		int y_pos = b_pos / MATRIX_WIDTH;

		if (y_pos < 0 || y_pos >= MATRIX_HEIGHT
		    || x_pos < 0 || x_pos >= MATRIX_WIDTH) 
			return false;
	}

	return true;
} 

/* funkcja zwraca ile lini zbito i usuwa zbite linie */
/* funkcja REKURENCYJNA, rekurencje zaprzegam zeby moc zbijac pelne linie,
 * pomimo tego ze sa przerwy niepelnych lini pomiedzy pelnymi. np:
 * |------|
 * |--- --|
 * |------|
 */
static int
clear_lines(struct game *g)
{
	struct square *row;
	int i, j;
	int line_s = -1, line_e;	/* poczatek i koniec pelnych lini */
	int count;			/* ile pelnych lini */

	/* sprawdzam linie od gory do dolu */
	for (count = i = 0; i < MATRIX_HEIGHT; ++i) {
		row = &g->m[i * MATRIX_WIDTH];
		/* sprawdzam czy linia jest pelna */
		for (j = 0; j < MATRIX_WIDTH; ++j)
			if (!row[j].visible)
				goto LINE_NOT_FULL;
		/* jesli petli nie przerwano wczesniej to j == MATRIX_WIDTH */
		++count;
		line_e = i;
		if (line_s == -1)
			line_s = i;

LINE_NOT_FULL:
		/* jesli znaleziono linie do zbicia a teraz jest linia nie
		 * do zbicia to przerywam petle */
		if (line_s != -1)
			break;
	}

	
	if (count) {
		/* przeniesienie lini z gory w dol */
		memmove(&g->m[count * MATRIX_WIDTH], g->m, sizeof(g->m[0]) 
		    * MATRIX_WIDTH * line_s);
#if 1		/* wyczyszczenie lini od gory */
		for (i = 0; i < count * MATRIX_WIDTH; ++i)
			g->m[i].visible = false;
#endif

		/* REKUNRENCJA: upewniam sie ze wszystko zbite -- jak liczba
		 * zbitych lini wynosi 0 to dopiero koncze dzialanie funkcj */
		count += clear_lines(g);
	}

	g->redraw(g);

	return count;
}

/* funkcja zwraca "roznice" w tetramino, tj. dostaje dwa tetramino jako 
   parametry i zwraca kafle do zamalowania na kolor tla i kafle nowego
   tetramino */
static struct update
update_tetramino(const int *old, const int *new, int o_size, int n_size)
{
	struct update u;
	/* wskazniki dla ulatwienia pisania... */
	int *color = u.square;
	int *background;
	int i, j;

	/* TODO: nie wysztkie kafle nowego trzeba zamalowywac... */
	/* wszystkie kafle nowego tetramino sa do zamalowania na kolor */
	for (i = 0; i < n_size; ++i)
		color[i] = new[i];

	u.color = n_size;

	background = &color[i];
	/* dopiero jak kafel starego tetramino jest rozny od kazdego kafla
	 * nowego tetramino to jest kaflem tla */
	for (u.background = i = 0; i < o_size; ++i) {
		for (j = 0; j < n_size; ++j)
			if (old[i] == new[j])
				goto SKIP;

		++u.background;
		*background++ = old[i];
SKIP:
		;
	}

	return u;
}

bool
check_squares(const struct game *g, const int *tmino, int size, int x, int y)
{
	int t_pos;			/* pozycja tetramino */
	int i;

	t_pos = x + y * MATRIX_WIDTH;
	for (i = 0; i < size; ++i) {
		int s_pos = tmino[i] + t_pos;

#ifdef DEBUG
		if (s_pos < 0 || s_pos >= MATRIX_SIZE) {
			fprintf(stderr, "%s:%d: tetramino points outside matrix"
			    ".\nAborting.\n", __FILE__, __LINE__);
			abort();
		}
#endif	/* DEBUG */
		
		if (g->m[s_pos].visible == true)
			return false;
	}

	return true;
}

static void
put_tetramino(struct game *g, const int *tmino, int size)
{
	int lines;
	int i;

	for (i = 0; i < size; ++i) {
		/* pozycja bloku */
		int b_pos = tmino[i] + g->x + g->y * MATRIX_WIDTH;

		g->m[b_pos].color = g->cur_tetramino;
		g->m[b_pos].visible = true;
	}
	
	/* punktacja i linie */
	g->lines_cleared += lines = clear_lines(g);
	g->points += g->level * lines * lines;

	/* zmiana levelu jesli zbito odpowiednia ilosc lini */
	if (g->lines_cleared >= LINES_PER_LEVEL * g->level) {
		if (g->levelup)
			g->levelup(g);

		g->redraw(g);
	}
}

/* TODO: to nie ma byc zaokraglanie do 10 tyko do MATRIX_WIDTH */
static int
round_to_ten(int num)
{
	return (num + 5) / 10 * 10;
}

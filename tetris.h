#ifndef _TETRIS_H_
#define _TETRIS_H_

#include <time.h>

#include "common.h"

#define MATRIX_WIDTH	10
#define MATRIX_HEIGHT	20
#define MATRIX_SIZE	MATRIX_HEIGHT * MATRIX_WIDTH
#define TETRAMINO_MAX	4
#define FALL_TIME	1200
#define FALL_BY_LEVEL	120
#define LINES_PER_LEVEL	10

/* kierunek przesuwania tetramino */
enum direction {
	LEFT = -1, NOWWHERE = 0, RIGHT = 1, DOWN = MATRIX_WIDTH
};
enum color {
	RED, GREY, CYAN, YELLOW, PURPLE, BLUE, GREEN, COLOR_COUNT
};
/* struktura opisujaca pojedynczy "kafel" planszy, okresla jego kolor oraz
 * to czy jest widoczny */
struct square {
	enum color color;
	bool visible;
    bool unvisited;
    bool selected;
    bool marked;
};
/* typ danych reprezentujacy plansze do gry, macierz struktur struct square */
typedef struct square Matrix[MATRIX_SIZE];
/* enumerator okreslajacy tetramino (klocki) */
enum tetra {TETRA_l, TETRA_T, TETRA_O, TETRA_L, TETRA_J, TETRA_S, TETRA_Z};
/* struktura pojedynczego tetramino na matriksie */
struct tetramino {
	int rotation[4][TETRAMINO_MAX];	/* 4 mozliwe rotacje tetramino */
	int rotations;			/* ile klocek ma rotacji */
	int size;			/* ile kafli zajmuje tetramino */
};

struct game {
	Matrix m;
	/* funkcja rysujaca matrix od nowa */
	void (*redraw)(struct game *);
	/* funkcja wywolywana przy przejsciu na nowy level */
	void (*levelup)(struct game *);
	void (*gameover)(struct game *);
	/* funkjca do przenoszenia tetramino na ekranie */
	void (*moved_tetramino)(struct game *, const int *, int, int);
	void (*preview)(struct game *, const int *, int);
	/* dane frontendu, np struktura z informacja o rzeczywistym polozeniu
	 * matriksu na ekranie */
	void *frontend;
	/* aktualne tetramino */
	enum tetra cur_tetramino;
	enum tetra next_tetramino;	/* kolejne tetramino jakie "wyskoczy" */
	time_t game_time;
	int x, y;			/* pozycja aktualnego tetramino */
	int rotation;			/* rotacja aktualnego tetramino */
	int next_rotation;		/* rotacja nastepnego tetramino */
	int lines_cleared;
	int points;
	int level;
	unsigned int fall_time;	        /* czas co jaki spada klocek */
	bool running;			/* czy gra trwa? */
    bool selected;
    bool marked;
    bool visible;
};

struct game *init_game(void (*)(struct game *), void (*)(struct game *),
    void (*)(struct game *), void (*)(struct game *, const int *, int, int),
    void (*)(struct game *, const int *, int), void *, int);
void restart_game(struct game *g);
void rotate_tetramino(struct game *, bool);
void move_tetramino(struct game *, enum direction);
void fast_forward(struct game *);
void new_tetramino(struct game *);
void redraw_all(struct game *);

const struct tetramino *get_tetramino(int selection);
bool check_bounds(const int *tmino, int len, int x, int y);
bool check_squares(const struct game *g, const int *tmino, int size, int x, int y);

#endif	/* _TETRIS_H_ */

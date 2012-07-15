#ifndef _HIGHSCORES_H_
#define _HIGHSCORES_H_

#include "common.h"

#define TOP_SCORES	10		/* ile wynikow na liste */
#define LINE_LEN	100		/* dlugosc lini w pliku */

/* jakie sa listy najlepszych wynikow */
/* TOP_COUNT to nie lista, to oznacza ile jest list, ma byc na koncu enuma */
enum top {TOP_LINES, TOP_POINTS, TOP_PPH, TOP_COUNT};

const char **get_top_scores(enum top);
bool high_score(int, int, int);
void write_scores(const char *, int, int, int);

#endif /* _HIGHSCORES_H_ */

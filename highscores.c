#include <stdio.h>
#include <string.h>
#include <time.h>

#include "common.h"
#include "highscores.h"
#include "xerror.h"

//#ifndef SCOREDIR
//#error "SCOREDIR is not defined!"
//#endif

#define DATE_LEN	26		/* czemu tyle? man ctime */

// yes, it can buffer-overflow, but whatever
char scoreString[LINE_LEN * TOP_SCORES];

/*
 * INFO: niechaj linia w pliku z najlepszymi wynikami wyglada tak:
 * imie_gracza	linie	punkty	pph
 * pph - oznacza punkty na godzine
 */

/* tablica z nazwami plikow odpowiednich list */
static const char *fnames[] = {
     "top_lines.lst",
     "top_points.lst",
     "top_pph.lst"
};

static bool string_to_scores(const char *, int *, int *, int *);
static void write_score_line(char *s, const char *, int, int, int);


/*
 * funkcja zwraca tablice stringow z najlepszymi wynikami, ostatni element
 * tablicy to NULL
 */
const char **
get_top_scores(enum top list)
{
    static char *scores[TOP_SCORES + 1];
    static char score_line[LINE_LEN * TOP_SCORES];
    char *s;
	int i;

	for (i = 0; i < TOP_SCORES; ++i)
		scores[i] = &score_line[i * LINE_LEN];

//	if ((f = fopen(fnames[list], "r")) == NULL) {
//		scores[0] = NULL;
//		return (const char **)scores;
//	}

	for (i = 0; i < TOP_SCORES; ++i)
		/* TODO: napisac jakas funkcje typu getline */
		if (fgets(scores[i], LINE_LEN, f) == NULL)
			break;
	
	fclose(f);
	
	scores[i] = NULL;
	return (const char **)scores;
}

/*
 * funkcja zwraca prawde jezeli wynik gracza kwalifikuje go na wpisanie na
 * ktorakolwiek z list.
 */
bool
high_score(int lines, int points, int pph)
{
	enum top t;

	for (t = TOP_LINES; t < TOP_COUNT; ++t) {
		const char **scores = get_top_scores(t);
		int c;			/* ile pozycji ma lista */

		/* petla po pozycjach danej listy */
		for (c = 0; *scores; ++scores, ++c) {
			const int *cur_score, *high_score;
			int hl, hp, hpph;

			if(!string_to_scores(*scores, &hl, &hp, &hpph))
				return true;

			switch (t) {
			case TOP_LINES:
				cur_score = &lines;
				high_score = &hl;
				break;
			case TOP_POINTS:
				cur_score = &points;
				high_score = &hp;
				break;
			case TOP_PPH:
				cur_score = &pph;
				high_score = &hpph;
				break;
			default:
				ERROR("unknow enumeration: %d\n", t);
			}

			if (*cur_score >= *high_score)
				return true;
		}

		/* jezeli lista nie jest jeszcze pelna to gracz lapie sie na
		 * nia automatycznie*/
		if (c < TOP_SCORES - 1)
			return true;
	}

	return false;
}
		
void
write_scores(const char *name, const int lines, const int points, const int pph)
{
    enum top t;
    int lcpy = lines, pcpy = points, pphcpy = pph;

    for (t = TOP_LINES; t < TOP_COUNT; ++t) {
        const char **scores = get_top_scores(t);
        int *cur_score;
        int score_count;

        if ((f = fopen(fnames[t], "w")) == NULL)
            ERROR("Can't open file for writing: %s\n", fnames[t]);

        if (scores[0] == NULL) {
            write_score_line(f, name, lines, points, pph);
            goto CNT;
        }

        for (score_count = 0; *scores; ++scores, ++score_count) {
            int hl, hp, hpph;
            int *high_score;

            if (!string_to_scores(*scores, &hl, &hp, &hpph))
                continue;

            switch (t) {
            case TOP_LINES:
                cur_score = &lcpy;
                high_score = &hl;
                break;
            case TOP_POINTS:
                cur_score = &pcpy;
                high_score = &hp;
                break;
            case TOP_PPH:
                cur_score = &pphcpy;
                high_score = &hpph;
                break;
            default:
                ERROR("unknow enumeration: %d\n", t);
            }

            /* jezeli aktualny wynik gracza jest lepszy
             * to wpisujemy go na to miejsce */
            if (*cur_score >= *high_score) {
                write_score_line(f, name, lines, points, pph);
                *cur_score = -1;
            }
			
            {
                char *eol = strchr(*scores, '\n');
                if (eol)
                    *eol = '\0';
            }
            fprintf(f, "%s\n", *scores);
        } /* inner lop */

        /* jesli lista jest krotsza niz TOP_SCORES i aktualny
         * wynik nie byl jeszcze zapisany to go zapisuje */
        if (score_count < TOP_SCORES && *cur_score > -1)
            write_score_line(f, name, lines, points, pph);

CNT:
        fclose(f);
    }
}
			
static bool
string_to_scores(const char *str, int *lines, int *points, int *pph)
{
	const char *fmt = "%*s %d %d %d";

	if (sscanf(str, fmt, lines, points, pph) < 3) 
		return false;

	return true;
}

static void
write_score_line(char *s, const char *name, int lines, int points, int pph)
{
	char date[DATE_LEN + 1];
	time_t d;
				
	time(&d);
	strftime(date, DATE_LEN, "%Y-%m-%d", localtime(&d));
	fprintf(f, "%-20s %5d %5d %5d %s\n", name, lines, points, pph, date);
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <SDL/SDL.h>
#ifdef MAC_OS_X_VERSION_10_6
#include <SDL_ttf/SDL_ttf.h>
#else
#include <SDL/SDL_ttf.h>
#endif
#include "xerror.h"

#define FONT_SIZE	15
#define ASCII_BEGIN	32
#define ASCII_END	126
/* ile znakow */
#define NUM_CHAR	ASCII_END - ASCII_BEGIN
#define SF_PRINTF_BUF	50
#define SF_GETS_SIZE	50

#define MAX(x, y)	(x) > (y) ? (x) : (y)

static SDL_Surface *chars[NUM_CHAR];	/* ASCii */
static struct metrics {
	int advance;
	int max_y;
} met[NUM_CHAR];
static int ascent;			/* najwyzszy punkt w foncie */
static int descent;			/* najnizszy punkt w foncie */

void
init_font(const char *fp)
{
    TTF_Font *font;
    SDL_Color fg = {255, 255, 255, 0};
    int i;

    if (TTF_Init() == -1)
        ERROR("TTF_Init: %s\n", TTF_GetError());

    if ((font = TTF_OpenFont(fp, FONT_SIZE)) == NULL)
        ERROR("TTF_OpenFont: %s\nFont path: %s\n", TTF_GetError(), fp);

    for (i = ASCII_BEGIN; i < ASCII_END; ++i) {
        const int idx = i - ASCII_BEGIN;
        chars[idx] = TTF_RenderGlyph_Blended(font, i, fg);

        if (chars[idx] == NULL)
            ERROR("%s\nGlyph: %d = '%c'\n", TTF_GetError(), i, i);

        if (TTF_GlyphMetrics(font, i, NULL, NULL, NULL, &met[idx].max_y,
            &met[idx].advance) == -1)
            ERROR("%s\n", TTF_GetError());

        ascent = TTF_FontAscent(font);
        descent = TTF_FontDescent(font);
    }

    TTF_CloseFont(font);
    TTF_Quit();
}

/*
 * funkcja blituje napis "msg" na powierzchnie sf, prostokat r, musi zawierac
 * x i y, a w i h zostana odpowiednio obliczone, r->x i r->y wyznaczaja 
 * pozycje napisu na powierzchni
 */
void
sf_puts(SDL_Surface *sf, SDL_Rect *r, const char *msg)
{
	SDL_Rect glyph_pos;
	unsigned int i;
	int y = 0;
	int w = 0;			/* szerokosc najdluzszej lini */

	r->h = ascent - descent;
	glyph_pos.x = r->x;
	/* petla blitujaca wszystkie znaki */
	for (i = 0; i < strlen(msg); ++i) {
		const int idx = msg[i] - ASCII_BEGIN;
		SDL_Surface *glyph;

		/* nowa linia */
		if (msg[i] == '\n') {
			y += ascent - descent;
			r->h += ascent - descent;
			/* sprawdzam czy ta linia jest dluzsza od poprzedniej */
			w = MAX(w, glyph_pos.x - r->x);
			
			glyph_pos.x = r->x;
			continue;
		}

		/* czy literka ma swoj graficzny odpowiednik? */
		if (idx < 0 || idx >= NUM_CHAR)
			continue;

		glyph = chars[idx];
		glyph_pos.y = r->y + ascent - met[idx].max_y; 
		glyph_pos.y += y;
		glyph_pos.w = glyph->w;
		glyph_pos.h = glyph->h;

		SDL_BlitSurface(glyph, NULL, sf, &glyph_pos);
		glyph_pos.x += met[idx].advance;
	}

	/* szerokosc i wysokosc calego napisu */
	r->w = MAX(w, glyph_pos.x - r->x);
}

/*
 * Funkcja czyta z klawiatury znaki i wypisuje je po sobie oraz zapisuje je
 * do tablicy "buf", funkcja nie przeczyta nowego znaku jezeli nie zmiesci sie
 * on juz w polu wyznaczonym przez "r" lub w tablicy "buf" nie ma juz wiecej
 * miejsca na znak. Funkcja wczyta maksymalnie o jeden znak mniej niz
 * "buf_size".
 */
/* TODO: poprawic komentarz powyzej i zrobic test na temat SF_GETS_SIZE */
SDL_Rect
sf_gets(SDL_Surface *bg, SDL_Rect *r, char * const buf, int buf_size)
{
	extern SDL_Surface *screen;
	int ch_pos[SF_GETS_SIZE + 1];	/* pozycja kazdego znaku */
	int *cpos = &ch_pos[1];		/* bierzaca pozycja znaku */
	SDL_Rect glyph_pos = {r->x};
	SDL_Rect ret;
	SDL_Event e;
	char *bufp = buf;	        /* wskaznik na bierzaca litere */

	ch_pos[0] = r->x;
	ret.x = r->x;
	ret.y = r->y;
	SDL_EnableUNICODE(1);
	for (;;) {
		SDL_Surface *glyph;
		int ch;			/* znak wcisnietego klawisza */
		int idx;

		if (SDL_PollEvent(&e) == 0)
			goto CNT;
		
		if (e.type != SDL_KEYDOWN)
			goto CNT;

		/* w tym miejscu napewno wiem ze wcisnieto klawisz */
		switch (e.key.keysym.sym) {
		case SDLK_RETURN:
			*bufp = '\0';
			return ret;
		case SDLK_BACKSPACE:	/* cofanie kursora */
			if (cpos >= &ch_pos[2]) {
				SDL_Rect bg_r;

				/* pozycja znaku do skasowania i ustawienie
				 * pozycj gdzie ma sie pojawic nowy znak */
				glyph_pos.x = bg_r.x = cpos[-2];
				bg_r.y = r->y;
				bg_r.w = cpos[-1] - cpos[-2];
				bg_r.h = ascent - descent;

				SDL_BlitSurface(bg, &bg_r, screen, &bg_r);
				SDL_UpdateRects(screen, 1, &bg_r);
								
				--cpos;
				--bufp;
			}
			goto CNT;
		default:
			/* sprawczam czy znak jest znakiem unicodu */
			if ((e.key.keysym.unicode & 0xFF80) != 0)
				goto CNT;
			;
		}

		ch = e.key.keysym.unicode & 0x7F;
		
		/* HACK: nick nie moze miec spacji, wiec tutaj sie tym zajme */
		if (ch == '\t' || ch == ' ')
			ch = '_';

		idx = ch - ASCII_BEGIN;

		if (idx < 0 || idx >= NUM_CHAR)
			goto CNT;

		/* straznik buforu */
		if (bufp >= &buf[buf_size - 1])
			goto CNT;
		
		*bufp++ = ch;

		glyph = chars[idx];
		glyph_pos.y = r->y + ascent - met[idx].max_y; 
		glyph_pos.w = glyph->w;
		glyph_pos.h = glyph->h;

		SDL_BlitSurface(glyph, NULL, screen, &glyph_pos);
		SDL_UpdateRects(screen, 1, &glyph_pos);
		glyph_pos.x += met[idx].advance;
		*cpos++ = glyph_pos.x;

CNT:
		SDL_Delay(5);
	}
	SDL_EnableUNICODE(0);
}

void
sf_printf(SDL_Surface *sf, SDL_Rect *r, const char *fmt, ...)
{
	char buf[SF_PRINTF_BUF];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buf, SF_PRINTF_BUF, fmt, ap);
	va_end(ap);

	sf_puts(sf, r, buf);
}

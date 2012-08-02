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

#include <SDL/SDL.h>

#include "game.h"
//#include "highscores.h"
#include "mainloop.h"
#include "tetris.h"
#include "text.h"

// My addons
#include "autoplay.h"
#include "mouseinput.h"
#include "copypaste.h"

#define DELAY	5

bool controlPressed = false;

extern SDL_Surface *screen;

static int event_filter(const SDL_Event *);
static void handle_keyboard(const SDL_keysym *, struct game *);
static void handle_fakekeyboard(const SDL_keysym *, struct game *);
static void handle_keyboardUnpressed(const SDL_keysym *, struct game *);
static void handle_mousedown(struct game *);
static void handle_mousemove(struct game *);

void
main_loop(struct game *g)
{
    SDL_Event e;
    Uint32 time;

    SDL_SetEventFilter(event_filter);
    new_tetramino(g);
    time = SDL_GetTicks();
    do {
        if (SDL_PollEvent(&e))
            switch (e.type) {
            case SDL_KEYDOWN:
                handle_keyboard(&e.key.keysym, g);
                break;
            case SDL_KEYUP:
                handle_keyboardUnpressed(&e.key.keysym, g);
                break;
            case SDL_MOUSEBUTTONDOWN:
                handle_mousedown(g);
                break;
            case SDL_MOUSEMOTION:
                handle_mousemove(g);
                break;
            case SDL_QUIT:
                return;
            }

        if (SDL_GetTicks() >= time + g->fall_time) {
            move_tetramino(g, DOWN);
            time = SDL_GetTicks();
        }

        SDL_keysym key;
        if (autoplay_update(&key))
            handle_fakekeyboard(&key, g);

#ifdef	CPU_SAVER
        SDL_Delay(DELAY);
#endif	/* CPU_SAVER */
    } while (g->running);
}

static void
handle_keyboard(const SDL_keysym *k, struct game *g)
{
    enum direction d = NOWWHERE;

    switch (k->sym) {
    case SDLK_RCTRL:
    case SDLK_LCTRL:
        controlPressed = true;
        break;
    case SDLK_x:
        if (controlPressed)
            manage_cut(g);
        break;
    case SDLK_c:
        if (controlPressed)
            manage_copy(g);
        break;
    case SDLK_v:
        if (controlPressed)
            manage_paste(g);
        break;
    case SDLK_s:
        restart_game(g);
        return;
    case SDLK_f:
        SDL_WM_ToggleFullScreen(screen);
        break;
//    case SDLK_h:
//        draw_hscores(g);
//        return;
#ifdef DEBUG
    case SDLK_a:
        fprintf(stderr, "Abort requested by user.\nAborting.\n");
        SDL_SaveBMP(screen, "tetris_dump.bmp");
        g->redraw(g);
        SDL_SaveBMP(screen, "tetris_dump2.bmp");
        abort();
        return;
#endif	/* DEBUG */
    case SDLK_q:
    case SDLK_ESCAPE:
        g->running = false;
        return;
    default:			/* glupoty powciskane */
        return;
    }

    move_tetramino(g, d);
}


static void 
handle_fakekeyboard(const SDL_keysym *k, struct game *g)
{
    enum direction d = NOWWHERE;

    switch (k->sym) {
    case SDLK_LEFT:
        d = LEFT;
        break;
    case SDLK_RIGHT:
        d = RIGHT;
        break;
//    case SDLK_RCTRL:
//        d = DOWN;
//        break;
    case SDLK_UP:
        rotate_tetramino(g, true);
        return;
    case SDLK_DOWN:
        rotate_tetramino(g, false);
        return;
    case SDLK_SPACE:
        fast_forward(g);
        return;
    default:			/* glupoty powciskane */
        return;
    }

    move_tetramino(g, d);
}

static void handle_keyboardUnpressed(const SDL_keysym *k, struct game *g)
{

    switch (k->sym) {
    case SDLK_RCTRL:
    case SDLK_LCTRL:
        controlPressed = false;
        break;
    default:
        break;
    }
}

static void
handle_mousedown(struct game *g)
{
    mouseClicked(g);
}

static void
handle_mousemove(struct game *g)
{
    mouseHover(g);
}

static int
event_filter(const SDL_Event *e)
{

    switch (e->type) {
    case SDL_QUIT:
        exit(EXIT_SUCCESS);
        /* NOTREACHED */
    }
}


//void
//draw_hscores(struct game *g)
//{
//    extern SDL_Surface *bground;
//    SDL_Rect pos;
//    const char **text;
////    enum top t;

//    pos.x = 100;

//    for (t = TOP_LINES;; t + 1 >= TOP_COUNT ? t = TOP_LINES : ++t) {
//        const char *caption[TOP_COUNT] = {
//            "Highest line scores:",
//            "Best points:",
//            "Best speed:"
//        };
//        /* format taki jak linia wyniku (funkcja write_score_line) */
//        const char *fmt = "%-20s %4s %4s %4s %s\n";
//        SDL_Event e;

//        /* czyszcze wszystko */
//        SDL_BlitSurface(bground, NULL, screen, NULL);

//        pos.y = 100;
//        sf_printf(screen, &pos, fmt, caption[t], "lines", "points",
//                  "pph", "date");

//        pos.y += pos.h;
//        for (text = get_top_scores(t); *text; ++text) {
//            char *eol;

//            if (eol = strchr(*text, '\n'))
//                *eol = '\0';

//            sf_puts(screen, &pos, *text);
//            pos.y += pos.h;
//        }
//        SDL_Flip(screen);

//        for (;;) {
//            if (SDL_PollEvent(&e) && e.type == SDL_KEYDOWN)
//                switch(e.key.keysym.sym) {
//                case SDLK_s:
//                    restart_game(g);
//                    goto RET;
//                case SDLK_f:
//                    SDL_WM_ToggleFullScreen(screen);
//                    break;
//                case SDLK_ESCAPE:
//                case SDLK_q:
//                    goto RET;
//                default:
//                    goto CNT;
//                }
//            SDL_Delay(5);
//        }
//CNT:
//        ;
//    } /* for */
//RET:
//    SDL_BlitSurface(bground, NULL, screen, NULL);
//    SDL_Flip(screen);
//    redraw_all(g);
//}

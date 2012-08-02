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

#include "mouseinput.h"

#include "tetris.h"
#include "game.h"

#include "copypaste.h"

#include "SDL/SDL.h"

static bool check_fallingtmino(struct game *g, int x, int y);
static bool check_background(struct game *g, int x, int y);
static bool recurse_bg(struct game *g, int x, int y, int c);

bool get_mousecoords(struct game *g, int *clickedx, int *clickedy)
{

    int mousex, mousey;
    SDL_GetMouseState(&mousex, &mousey);

    // find if I'm clicking a block
    struct position *rect = g->frontend;

    if (mousex < rect->x || mousey < rect->y)
        return false;

    *clickedx = (mousex - rect->x) / rect->size;
    *clickedy = (mousey - rect->y) / rect->size;

    if (*clickedx >= MATRIX_WIDTH || *clickedy >= MATRIX_HEIGHT)
        return false;
    return true;
}

void mouseClicked(struct game *g)
{
    int clickedx, clickedy;

    if (!get_mousecoords(g, &clickedx, &clickedy))
        return;

    if (g->marked) {
        unselect_background(g);
        select_tmino(g, clickedx, clickedy);
    } else if (mark_valid(g)) {
        unselect_tmino(g);
        move_mark_to_selection(g);
    } else {
        unselect_tmino(g);
        unselect_background(g);
    }

    redraw_field(g);
}

int *get_tetraminosquares(struct game *g)
{
    const struct tetramino *t = get_tetramino(g->cur_tetramino);
    return t->rotation[g->rotation];
}

void mouseHover(struct game *g)
{
    int clickedx, clickedy;

    if (!get_mousecoords(g, &clickedx, &clickedy)) {
        change_preview_visible(g, false);
        g->marked = false;
        redraw_field(g);
        return;
    }

    // falling tetramino
    g->marked = false;
    if (check_fallingtmino(g, clickedx, clickedy)) {
        mark_tmino(g, clickedx, clickedy);
        unmark_background(g);
        change_preview_visible(g, false);
    } else if (!fits_preview(g, clickedx, clickedy) && check_background(g, clickedx, clickedy)) {
        unmark_background(g);
        mark_background(g, clickedx, clickedy);
        change_preview_visible(g, false);
    } else {
        unmark_background(g);
        change_preview_visible(g, true);
    }
    redraw_field(g);
}

void recheckMouse(struct game *g)
{
    int clickedx, clickedy;

    if (!get_mousecoords(g, &clickedx, &clickedy))
        return;

    // falling tetramino
    g->marked = false;
    if (check_fallingtmino(g, clickedx, clickedy)) {
        mark_tmino(g, clickedx, clickedy);
        change_preview_visible(g, false);
    } else if (!fits_preview(g, clickedx, clickedy) && check_background(g, clickedx, clickedy)) {
        mark_background(g, clickedx, clickedy);
        change_preview_visible(g, false);
    } else {
        unmark_background(g);
        change_preview_visible(g, true);
    }
    redraw_field(g);
}

static bool check_fallingtmino(struct game *g, int x, int y)
{
    const struct tetramino *t = get_tetramino(g->cur_tetramino);
    int mousepos = x + y * MATRIX_WIDTH;
    int t_pos = g->x + g->y * MATRIX_WIDTH;
    int i;

    int *tmino_squares = t->rotation[g->rotation];

    for (i=0; i< t->size; i++)
        if (mousepos == t_pos + tmino_squares[i]) {
            return true;
            break;
        }

    return false;
}

static bool check_background(struct game *g, int x, int y)
{
    int pos = x + y * MATRIX_WIDTH;
    return g->m[pos].visible;
}


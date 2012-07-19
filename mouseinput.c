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


    // falling tetramino
    if (check_fallingtmino(g, clickedx, clickedy)) {
        select_tmino(g, clickedx, clickedy);
//        redraw_all(g);
    } else if (check_background(g, clickedx, clickedy)) {
        select_background(g, clickedx, clickedy);
    }
}

int *get_tetraminosquares(struct game *g)
{
    const struct tetramino *t = get_tetramino(g->cur_tetramino);
    return t->rotation[g->rotation];
}

void mouseHover(struct game *g)
{
    int clickedx, clickedy;

    if (!get_mousecoords(g, &clickedx, &clickedy))
        return;

    // falling tetramino
    g->marked = false;
    if (check_fallingtmino(g, clickedx, clickedy)) {
        mark_tmino(g, clickedx, clickedy);
        undraw_preview(g);
        draw_tetramino(g, get_tetraminosquares(g), 0, 4);
//        redraw_all(g);
    } else if (check_background(g, clickedx, clickedy)) {
        undraw_preview(g);
        mark_background(g, clickedx, clickedy);
        draw_tetramino(g, get_tetraminosquares(g), 0, 4);
//        draw_mark(g);
    } else {
        draw_preview(g, false);
        draw_tetramino(g, get_tetraminosquares(g), 0, 4);
    }
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
    } else if (check_background(g, clickedx, clickedy)) {
        mark_background(g, clickedx, clickedy);
    } else {
        draw_preview(g, true);
        // still, the moving tetramino will undraw itself here...
        // I think I should start doing the layer stuff

//        draw_tetramino(g, get_tetraminosquares(g), 0, 4);
    }
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


#include "copypaste.h"
#include "mouseinput.h"

#include "tetris.h"
#include "game.h"

typedef struct {
    int color;
    int pos[MATRIX_SIZE];
    int len;
    int ox, oy;
    bool isTetramino;
    bool valid;
} blockinfo;

blockinfo Buffer;
blockinfo Selection;
blockinfo Mark;

static void push_tetramino(struct game * g);
static void pop_tetramino(struct game *g, int x, int y);
static bool can_pop_tetramino(struct game *g, int x, int y);
static void prepare_selection(struct game *g);
static void recurse_bg(struct game *g, int x, int y);
static void normalize_selection();
static void unselect_background(struct game *g);
static void move_selection_to_buffer(struct game *g);
static void del_selection(struct game *g);
static bool
custom_check_bounds(const int *tmino, int len, int x, int y);
static void custom_check_lines(struct game *g, int destx, int desty);
static void unmark_background(struct game *g);

void restart_buffer(struct game *g)
{
    Buffer.len = 0;
    Buffer.isTetramino = false;

    int i;

    for (i=0; i<MATRIX_SIZE; i++) {
        g->m[i].unvisited = true;
        g->m[i].selected = false;
        g->m[i].marked = false;
    }
}

void manage_copy(struct game *g)
{
    if (g->selected) {
        push_tetramino(g);
        g->selected = false;
    } else if (Selection.len > 0) {
        move_selection_to_buffer(g);
        unselect_background(g);
        redraw_all(g);
    }
}

void manage_paste(struct game *g)
{
    int mousex, mousey;
    if (!get_mousecoords(g, &mousex, &mousey))
        return;

    if (can_pop_tetramino(g, mousex, mousey)) {
        pop_tetramino(g, mousex, mousey);
        custom_check_lines(g, mousex, mousey);
        redraw_all(g);
    }
}

void manage_cut(struct game *g)
{
    if (g->selected) {
        g->redraw(g);
        push_tetramino(g);
        new_tetramino(g);
    } else if (Selection.len > 0) {
        move_selection_to_buffer(g);
        del_selection(g);
        redraw_all(g);
    }
}

static void push_tetramino(struct game * g)
{
    const struct tetramino *t = get_tetramino(g->cur_tetramino);
    int i;

    int *tmino_squares = t->rotation[g->rotation];

    Buffer.len = t->size;
    for (i=0; i< Buffer.len; i++) {
        Buffer.pos[i] = tmino_squares[i];
        Buffer.color = g->cur_tetramino;
    }

    Buffer.isTetramino = true;
    Buffer.ox = -1;
    Buffer.oy = -1;

//    fflush(0);
}

static bool can_pop_tetramino(struct game *g, int x, int y)
{
    if (Buffer.len == 0)
        return false;

    // check bounds
    if (!custom_check_bounds(Buffer.pos, Buffer.len, x, y))
    {
        return false;
    }

    // check collision
    if (!check_squares(g, Buffer.pos, Buffer.len, x, y))
    {
        return false;
    }

    return true;
}

static void pop_tetramino(struct game *g, int x, int y)
{
    if (Buffer.len == 0)
        return;

    int mpos = x + y * MATRIX_WIDTH;
    int i;
    for (i = 0; i < Buffer.len; i++) {
        g->m[mpos + Buffer.pos[i]].color = Buffer.color;
        g->m[mpos + Buffer.pos[i]].visible = true;
    }
}

void select_tmino(struct game *g, int x, int y)
{
    g->selected = true;
}

void select_background(struct game *g, int x, int y)
{
    g->selected = false;
    unselect_background(g);

    int pos = x + y * MATRIX_WIDTH;
    if (!g->m[pos].visible) {
        redraw_all(g);
        return;
    }

    Selection.color = g->m[pos].color;
//    Selection.isTetramino = false;
    prepare_selection(g);
    recurse_bg(g, x, y);
    normalize_selection();
    redraw_all(g);
}

static void unselect_background(struct game *g)
{
    if (Selection.len == 0)
        return;
    // go through the background
    int i;
    for (i=0; i<Selection.len; i++) {
        int x = Selection.pos[i] % MATRIX_WIDTH + Selection.ox;
        int y = Selection.pos[i] / MATRIX_WIDTH + Selection.oy;
        int pos = x + y * MATRIX_WIDTH;
        g->m[pos].selected = false;
    }
    Selection.len = 0;
}

static void recurse_bg(struct game *g, int x, int y)
{
    if (x < 0 || x >= MATRIX_WIDTH || y < 0 || y > MATRIX_HEIGHT)
        return;

    int pos = x + y * MATRIX_WIDTH;
    if (g->m[pos].color == Selection.color && g->m[pos].visible && g->m[pos].unvisited) {
        Selection.pos[Selection.len++] = pos;
        g->m[pos].unvisited = false;
        g->m[pos].selected = true;
        recurse_bg(g, x-1, y);
        recurse_bg(g, x+1, y);
        recurse_bg(g, x, y-1);
        recurse_bg(g, x, y+1);
    }
}

static void recurse_bg_mark(struct game *g, int x, int y)
{
    if (x < 0 || x >= MATRIX_WIDTH || y < 0 || y > MATRIX_HEIGHT)
        return;

    int pos = x + y * MATRIX_WIDTH;
    if (g->m[pos].color == Mark.color && g->m[pos].visible && g->m[pos].unvisited) {
        Mark.pos[Mark.len++] = pos;
        g->m[pos].unvisited = false;
        g->m[pos].marked = true;
        recurse_bg(g, x-1, y);
        recurse_bg(g, x+1, y);
        recurse_bg(g, x, y-1);
        recurse_bg(g, x, y+1);
    }
}

static void prepare_selection(struct game *g)
{
    int i;
    for (i=0; i< MATRIX_SIZE; i++)
        g->m[i].unvisited = true;
}

static void normalize_selection()
{
    int minx, maxx, miny, maxy;
    int i;

    if (Selection.len == 0)
        return;

    minx = Selection.pos[0] % MATRIX_WIDTH;
    miny = Selection.pos[0] / MATRIX_WIDTH;
    maxx = minx;
    maxy = miny;
    for (i = 1; i < Selection.len; i++) {
        int x = Selection.pos[i] % MATRIX_WIDTH;
        int y = Selection.pos[i] / MATRIX_WIDTH;
        if (x < minx)
            x = minx;
        if (x > maxx)
            x = maxx;
        if (x < miny)
            y = miny;
        if (y > maxy)
            y = maxy;
    }
    Selection.ox = (minx + maxx) / 2;
    Selection.oy = (miny + maxy) / 2;
    for (i = 0; i < Selection.len; i++) {
        int x = Selection.pos[i] % MATRIX_WIDTH - Selection.ox;
        int y = Selection.pos[i] / MATRIX_WIDTH - Selection.oy;
        Selection.pos[i] = x + y * MATRIX_WIDTH;
    }
}

static void normalize_mark()
{
    int minx, maxx, miny, maxy;
    int i;

    if (Mark.len == 0)
        return;

    minx = Mark.pos[0] % MATRIX_WIDTH;
    miny = Mark.pos[0] / MATRIX_WIDTH;
    maxx = minx;
    maxy = miny;
    for (i = 1; i < Mark.len; i++) {
        int x = Mark.pos[i] % MATRIX_WIDTH;
        int y = Mark.pos[i] / MATRIX_WIDTH;
        if (x < minx)
            x = minx;
        if (x > maxx)
            x = maxx;
        if (x < miny)
            y = miny;
        if (y > maxy)
            y = maxy;
    }
    Mark.ox = (minx + maxx) / 2;
    Mark.oy = (miny + maxy) / 2;
    for (i = 0; i < Mark.len; i++) {
        int x = Mark.pos[i] % MATRIX_WIDTH - Mark.ox;
        int y = Mark.pos[i] / MATRIX_WIDTH - Mark.oy;
        Mark.pos[i] = x + y * MATRIX_WIDTH;
    }
}


static void move_selection_to_buffer(struct game *g)
{
    int i;
    for (i=0; i < Selection.len; i++)
        Buffer.pos[i] = Selection.pos[i];
    Buffer.isTetramino = false;
    Buffer.color = Selection.color;
    Buffer.len = Selection.len;
    Buffer.ox = -1;
    Buffer.oy = -1;
}

static void move_mark_to_selection(struct game *g)
{
    int i;
    for (i=0; i < Mark.len; i++)
        Selection.pos[i] = Mark.pos[i];
    Selection.isTetramino = false;
    Selection.color = Mark.color;
    Selection.len = Mark.len;
    Selection.ox = -1;
    Selection.oy = -1;
}

static void del_selection(struct game *g)
{
    if (Selection.len == 0)
        return;
    // go through the background
    int i;
    for (i=0; i<Selection.len; i++) {
        int x = Selection.pos[i] % MATRIX_WIDTH + Selection.ox;
        int y = Selection.pos[i] / MATRIX_WIDTH + Selection.oy;
        int pos = x + y * MATRIX_WIDTH;
        g->m[pos].selected = false;
        g->m[pos].visible = false;
    }
    Selection.len = 0;
}

static bool
custom_check_bounds(const int *tmino, int len, int x, int y)
{
    // they do something strange with the horizontal position,
    // and the comments are in some eastern european language
    // czech? polish?  Whatever, I just do it myself
    // instead of figuring out that

    int i;
    const int t_pos = x + y * MATRIX_WIDTH;

    if (x < 0 || x > MATRIX_WIDTH || y < 0 || y > MATRIX_HEIGHT)
        return false;

    for (i = 0; i < len; ++i) {
        int x_tmp = tmino[i] % MATRIX_WIDTH;
        int x_pos = x_tmp - MATRIX_WIDTH * ((x_tmp*2)/MATRIX_WIDTH) + x;
        int y_pos = tmino[i] / MATRIX_WIDTH + y;


        if (y_pos < 0 || y_pos >= MATRIX_HEIGHT
            || x_pos < 0 || x_pos >= MATRIX_WIDTH) {
            return false;
        }
    }

    return true;
}

static void custom_check_lines(struct game *g, int destx, int desty)
{
    if (Buffer.len == 0)
        return;

    // find range
    int miny = Buffer.pos[0] / MATRIX_WIDTH;
    int maxy = Buffer.pos[0] / MATRIX_WIDTH;
    int i;
    for (i=1; i<Buffer.len; i++) {
        int y = Buffer.pos[i] / MATRIX_WIDTH;
        if (y < miny)
            miny = y;
        if (y > maxy)
            maxy = y;
    }

    miny += desty;
    maxy += desty;

    int lines = 0;

    // check the lines
    for (i=miny; i<= maxy; i++) {
        int j;
        for (j=0; j<MATRIX_WIDTH;j++)
            if (!g->m[j + i*MATRIX_WIDTH].visible)
                break;
        if (j == MATRIX_WIDTH) {
            // line killed
            lines++;
            for (j=0; j<MATRIX_WIDTH;j++) {
                g->m[j + i*MATRIX_WIDTH].visible = false;
                g->m[j + i*MATRIX_WIDTH].selected = false;
            }
        }
    }

    g->lines_cleared += lines;
    g->points += g->level * lines * lines;

    /* zmiana levelu jesli zbito odpowiednia ilosc lini */
    if (g->lines_cleared >= LINES_PER_LEVEL * g->level) {
        if (g->levelup)
            g->levelup(g);
    }
}

void draw_preview(struct game *g)
{
    // no preview
    if (Buffer.len == 0)
        return;

    // out of screen
    int mx, my;
    if (!get_mousecoords(g, &mx, &my))
        return;

    // new location: check if it's valid
    if (mx != Buffer.ox || my != Buffer.oy) {
        redraw_all(g);
        Buffer.valid = can_pop_tetramino(g, mx,my);
        Buffer.ox = mx;
        Buffer.oy = my;

        // drawing code
        int i;
        for (i = 0; i < Buffer.len; i++) {
            int x = Buffer.pos[i] % MATRIX_WIDTH + Buffer.ox;
            int y = Buffer.pos[i] / MATRIX_WIDTH + Buffer.oy;
            if (x < 0 || x >= MATRIX_WIDTH || y < 0 || y >= MATRIX_HEIGHT)
                continue;
            draw_block(g, x, y, Buffer.color, Buffer.valid);
        }
    }
}

void draw_mark(struct game *g)
{
    // no preview
    if (Mark.len == 0)
        return;

    // new location: check if it's valid
    redraw_all(g);

    // drawing code
    int i;
    for (i = 0; i < Mark.len; i++) {
        int x = Mark.pos[i] % MATRIX_WIDTH + Mark.ox;
        int y = Mark.pos[i] / MATRIX_WIDTH + Mark.oy;
        if (x < 0 || x >= MATRIX_WIDTH || y < 0 || y >= MATRIX_HEIGHT)
            continue;
        draw_block_mark(g, x, y, Mark.color);
    }
}

void mark_tmino(struct game *g, int x, int y)
{
    g->marked = true;
}

void mark_background(struct game *g, int x, int y)
{
    g->selected = false;
    unmark_background(g);

    int pos = x + y * MATRIX_WIDTH;

    Mark.color = g->m[pos].color;
    if (Mark.color > 10) {
        printf("got %i in %i(%i,%i)\n",Mark.color,pos,x,y);
        fflush(0);
    }
//    Mark.isTetramino = false;
    prepare_selection(g);
    recurse_bg_mark(g, x, y);
//    normalize_selection();
    Mark.ox = 0;
    Mark.oy = 0;
    redraw_all(g);
}

static void unmark_background(struct game *g)
{
    if (Mark.len == 0)
        return;
    // go through the background
    int i;
    for (i=0; i<Mark.len; i++) {
        int x = Mark.pos[i] % MATRIX_WIDTH + Mark.ox;
        int y = Mark.pos[i] / MATRIX_WIDTH + Mark.oy;
        int pos = x + y * MATRIX_WIDTH;
        g->m[pos].marked = false;
    }
    Mark.len = 0;
}

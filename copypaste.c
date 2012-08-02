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

#include "copypaste.h"
#include "mouseinput.h"

#include "tetris.h"
#include "game.h"

typedef struct {
    int color;
    int pos[MATRIX_SIZE];
    int len;
    int x_topleft, y_topleft;
    int x_middle, y_middle;
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
static void normalize_buffer();
//static void unselect_background(struct game *g);
static void move_selection_to_buffer(struct game *g);
//static void del_selection(struct game *g);
static bool
custom_check_bounds(const int *tmino, int len, int x, int y);
static void custom_check_lines(struct game *g, int destx, int desty);
static void move_buffer(blockinfo *bufferFrom, blockinfo *bufferTo);
static void set_selection(struct game *g, bool selected);
//static void unmark_background(struct game *g);
static void cut_selection(struct game *g);

bool preview_visible;

void clean_buffers(struct game *g)
{
    Buffer.len = 0;
    Selection.len = 0;
    Mark.len = 0;

    int i;
    for (i=0; i<MATRIX_SIZE; i++) {
        g->m[i].unvisited = true;
        g->m[i].selected = false;
        g->m[i].marked = false;
    }
    g->selected = false;
    g->marked = false;
}

void manage_copy(struct game *g)
{
    undraw_preview(g);
    if (g->selected) {
        push_tetramino(g);
        g->selected = false;
    } else if (Selection.len > 0) {
        move_selection_to_buffer(g);
        set_selection(g, false);
    }
    call_updaterects(g, 0, 0, MATRIX_WIDTH, MATRIX_HEIGHT);
}

void manage_paste(struct game *g)
{
    int mousex, mousey;
    if (!get_mousecoords(g, &mousex, &mousey))
        return;

    if (can_pop_tetramino(g, mousex, mousey)) {
        pop_tetramino(g, mousex, mousey);
        custom_check_lines(g, mousex, mousey);
    }
}

void manage_cut(struct game *g)
{
    undraw_preview(g);
    if (g->selected) {
        push_tetramino(g);
        g->visible = false;
        draw_tetramino(g, get_tetraminosquares(g), 0, 4);
        new_tetramino(g);
    } else if (Selection.len > 0) {
        move_selection_to_buffer(g);
        set_selection(g, false);
        cut_selection(g);
    }
    call_updaterects(g, 0, 0, MATRIX_WIDTH, MATRIX_HEIGHT);
}

static void push_tetramino(struct game * g)
{
    const struct tetramino *t = get_tetramino(g->cur_tetramino);
    int i;

    int *tmino_squares = t->rotation[g->rotation];

    Buffer.len = t->size;
    for (i=0; i< Buffer.len; i++) {
        Buffer.pos[i] = tmino_squares[i] + 5;
        Buffer.color = g->cur_tetramino;
    }

    normalize_buffer();
    Buffer.x_topleft = -1;
    Buffer.y_topleft = -1;
}

static bool can_pop_tetramino(struct game *g, int x, int y)
{
    if (Buffer.len == 0)
        return false;

    x-=Buffer.x_middle;
    y-=Buffer.y_middle;
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

    x-= Buffer.x_middle;
    y-= Buffer.y_middle;
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

void unselect_tmino(struct game *g)
{
    g->selected = false;
}

void unselect_background(struct game *g)
{
    set_selection(g, false);
    Selection.len = 0;
}

//void select_background(struct game *g, int x, int y)
//{
//    g->selected = false;
//    unselect_background(g);

//    int pos = x + y * MATRIX_WIDTH;
//    if (!g->m[pos].visible) {
//        return;
//    }

//    Selection.color = g->m[pos].color;
//    prepare_selection(g);
//    recurse_bg(g, x, y);
//    normalize_selection();
//    redraw_all(g);
//}

//void unselect_background(struct game *g)
//{
//    if (Selection.len == 0)
//        return;
//    // go through the background
//    int i;
//    for (i=0; i<Selection.len; i++) {
//        int x = Selection.pos[i] % MATRIX_WIDTH + Selection.x_topleft;
//        int y = Selection.pos[i] / MATRIX_WIDTH + Selection.y_topleft;
//        int pos = x + y * MATRIX_WIDTH;
//        g->m[pos].selected = false;
//    }
//    Selection.len = 0;
//}

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
    if (x < 0 || x >= MATRIX_WIDTH || y < 0 || y >= MATRIX_HEIGHT) {
        return;
    }

    int pos = x + y * MATRIX_WIDTH;
    if (g->m[pos].color == Mark.color && g->m[pos].visible && g->m[pos].unvisited) {
        Mark.pos[Mark.len++] = pos;
        g->m[pos].unvisited = false;
        g->m[pos].marked = true;
        recurse_bg_mark(g, x-1, y);
        recurse_bg_mark(g, x+1, y);
        recurse_bg_mark(g, x, y-1);
        recurse_bg_mark(g, x, y+1);
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
            minx = x;
        if (x > maxx)
            maxx = x;
        if (y < miny)
            miny = y;
        if (y > maxy)
            maxy = y;
    }
    Selection.x_topleft = minx;
    Selection.y_topleft = miny;
    Selection.x_middle = (maxx - minx) / 2;
    Selection.y_middle = (maxy - miny) / 2;
    for (i = 0; i < Selection.len; i++) {
        int x = Selection.pos[i] % MATRIX_WIDTH - Selection.x_topleft;
        int y = Selection.pos[i] / MATRIX_WIDTH - Selection.y_topleft;
        Selection.pos[i] = x + y * MATRIX_WIDTH;
    }
}

static void normalize_buffer()
{
    int minx, maxx, miny, maxy;
    int i;

    if (Buffer.len == 0)
        return;

    minx = Buffer.pos[0] % MATRIX_WIDTH;
    miny = Buffer.pos[0] / MATRIX_WIDTH;
    maxx = minx;
    maxy = miny;
    for (i = 1; i < Buffer.len; i++) {
        int x = Buffer.pos[i] % MATRIX_WIDTH;
        int y = Buffer.pos[i] / MATRIX_WIDTH;
        if (x < minx)
            minx = x;
        if (x > maxx)
            maxx = x;
        if (y < miny)
            miny = y;
        if (y > maxy)
            maxy = y;
    }
    Buffer.x_topleft = minx;
    Buffer.y_topleft = miny;
    Buffer.x_middle = (maxx - minx) / 2;
    Buffer.y_middle = (maxy - miny) / 2;
    for (i = 0; i < Buffer.len; i++) {
        int x = Buffer.pos[i] % MATRIX_WIDTH - Buffer.x_topleft;
        int y = Buffer.pos[i] / MATRIX_WIDTH - Buffer.y_topleft;
        Buffer.pos[i] = x + y * MATRIX_WIDTH;
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
            minx = x;
        if (x > maxx)
            maxx = x;
        if (y < miny)
            miny = y;
        if (y > maxy)
            maxy = y;
    }
    Mark.x_topleft = minx;
    Mark.y_topleft = miny;
    Mark.x_middle = (maxx - minx) / 2;
    Mark.y_middle = (maxy - miny) / 2;
    for (i = 0; i < Mark.len; i++) {
        int x = Mark.pos[i] % MATRIX_WIDTH - Mark.x_topleft;
        int y = Mark.pos[i] / MATRIX_WIDTH - Mark.y_topleft;
        Mark.pos[i] = x + y * MATRIX_WIDTH;
    }
}


static void move_selection_to_buffer(struct game *g)
{
    move_buffer(&Selection, &Buffer);
    normalize_buffer();
//    int i;
//    for (i=0; i < Selection.len; i++)
//        Buffer.pos[i] = Selection.pos[i];
//    Buffer.color = Selection.color;
//    Buffer.len = Selection.len;
//    normalize_buffer();
//    Buffer.x_topleft = -1;
//    Buffer.y_topleft = -1;
}

//static void move_mark_to_selection(struct game *g)
//{
//    int i;
//    for (i=0; i < Mark.len; i++)
//        Selection.pos[i] = Mark.pos[i];
//    Selection.color = Mark.color;
//    Selection.len = Mark.len;
//    Selection.x_topleft = Mark.x_topleft;
//    Selection.y_topleft = Mark.y_topleft;
//}

//static void del_selection(struct game *g)
//{
//    if (Selection.len == 0)
//        return;
//    // go through the background
//    int i;
//    for (i=0; i<Selection.len; i++) {
//        int x = Selection.pos[i] % MATRIX_WIDTH + Selection.x_topleft;
//        int y = Selection.pos[i] / MATRIX_WIDTH + Selection.y_topleft;
//        int pos = x + y * MATRIX_WIDTH;
//        g->m[pos].selected = false;
//        g->m[pos].visible = false;
//    }
//    Selection.len = 0;
//}

static bool
custom_check_bounds(const int *tmino, int len, int x, int y)
{
    int i;

    if (x < 0 || x >= MATRIX_WIDTH || y < 0 || y >= MATRIX_HEIGHT)
        return false;

    for (i = 0; i < len; ++i) {
        int x_pos = tmino[i] % MATRIX_WIDTH + x;
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

    miny += desty - Buffer.y_middle;
    maxy += desty - Buffer.y_middle;

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

    if (lines > 0) {
        g->lines_cleared += lines;
        g->points += g->level * lines * lines;

        /* zmiana levelu jesli zbito odpowiednia ilosc lini */
        if (g->lines_cleared >= LINES_PER_LEVEL * g->level) {
            if (g->levelup)
                g->levelup(g);
        }
        const struct tetramino *t = get_tetramino(g->next_tetramino);
        if (g->preview)
            g->preview(g, t->rotation[g->next_rotation], t->size);
    }
}

void undraw_preview(struct game *g)
{
    if (Buffer.len == 0)
        return;

    int i;
    for (i = 0; i < Buffer.len; i++) {
        int x = Buffer.pos[i] % MATRIX_WIDTH + Buffer.x_topleft - Buffer.x_middle;
        int y = Buffer.pos[i] / MATRIX_WIDTH + Buffer.y_topleft - Buffer.y_middle;
        if (x < 0 || x >= MATRIX_WIDTH || y < 0 || y >= MATRIX_HEIGHT)
            continue;

        draw_field(g, x, y);
    }

    Buffer.x_topleft = -1;
    Buffer.y_topleft = -1;
}

void draw_preview(struct game *g, bool force)
{
    // no preview
    if (Buffer.len == 0)
        return;

    // out of screen
    int mx, my;
    if (!get_mousecoords(g, &mx, &my))
        return;

    // new location: check if it's valid
    if (force || mx != Buffer.x_topleft || my != Buffer.y_topleft) {
        undraw_preview(g);
        Buffer.valid = can_pop_tetramino(g, mx,my);
        Buffer.x_topleft = mx;
        Buffer.y_topleft = my;

        // drawing code
        int i;
        for (i = 0; i < Buffer.len; i++) {
            int x = Buffer.pos[i] % MATRIX_WIDTH + Buffer.x_topleft - Buffer.x_middle;
            int y = Buffer.pos[i] / MATRIX_WIDTH + Buffer.y_topleft - Buffer.y_middle;

            if (x < 0 || x >= MATRIX_WIDTH || y < 0 || y >= MATRIX_HEIGHT) {
                continue;
            }

            draw_block(g, x, y, Buffer.color, Buffer.valid);
        }
    }
}

//void undraw_mark(struct game *g)
//{
//    // no preview
//    if (Mark.len == 0)
//        return;

//    // drawing code
//    int i;
//    for (i = 0; i < Mark.len; i++) {
//        int x = Mark.pos[i] % MATRIX_WIDTH + Mark.x_topleft;
//        int y = Mark.pos[i] / MATRIX_WIDTH + Mark.y_topleft;

//        if (x < 0 || x >= MATRIX_WIDTH || y < 0 || y >= MATRIX_HEIGHT)
//            continue;
//        g->m[x+y*MATRIX_WIDTH].marked = false;
//        draw_field(g, x, y, Mark.color);
//    }
//    call_updaterects(g, 0,0,MATRIX_WIDTH,MATRIX_HEIGHT);
//}

//void draw_mark(struct game *g)
//{
//    // no preview
//    if (Mark.len == 0)
//        return;

//    // drawing code
//    int i;
//    for (i = 0; i < Mark.len; i++) {
//        int x = Mark.pos[i] % MATRIX_WIDTH + Mark.x_topleft;
//        int y = Mark.pos[i] / MATRIX_WIDTH + Mark.y_topleft;

//        if (x < 0 || x >= MATRIX_WIDTH || y < 0 || y >= MATRIX_HEIGHT)
//            continue;
//        draw_block_mark(g, x, y, Mark.color);
//    }
//    call_updaterects(g, 0,0,MATRIX_WIDTH,MATRIX_HEIGHT);
//}

void mark_tmino(struct game *g, int x, int y)
{
    g->marked = true;
    Mark.len = 0;
}

void mark_background(struct game *g, int x, int y)
{
//    g->selected = false;
    unmark_background(g);

    int pos = x + y * MATRIX_WIDTH;

    Mark.color = g->m[pos].color;
    prepare_selection(g);
    Mark.len = 0;
    recurse_bg_mark(g, x, y);
    normalize_mark();
}

void unmark_background(struct game *g)
{
    if (Mark.len == 0)
        return;
    // go through the background
    int i;
    for (i=0; i<Mark.len; i++) {
        int x = Mark.pos[i] % MATRIX_WIDTH + Mark.x_topleft;
        int y = Mark.pos[i] / MATRIX_WIDTH + Mark.y_topleft;

        int pos = x + y * MATRIX_WIDTH;
        g->m[pos].marked = false;
    }
    Mark.len = 0;
}

bool draw_preview_in_pos(struct game *g, int x, int y)
{
    int i;
    for (i=0; i<Buffer.len; i++) {
        int px = Buffer.pos[i] % MATRIX_WIDTH + Buffer.x_topleft - Buffer.x_middle;
        int py = Buffer.pos[i] / MATRIX_WIDTH + Buffer.y_topleft - Buffer.y_middle;
        if (px == x && py == y) {
            draw_block(g, x, y, Buffer.color, Buffer.valid);
            return true;
        }
    }
    return false;
}

void redraw_field(struct game *g)
{
    // no need to undraw/redraw shit
    int i;

    // first prepare visitation matrix
    for (i=0; i < MATRIX_SIZE; i++) {
        g->m[i].unvisited = true;
    }

    // cut out falling tetramino
    const struct tetramino *t = get_tetramino(g->cur_tetramino);
    int *tmino_squares = t->rotation[g->rotation];
    int mpos = g->x + g->y * MATRIX_WIDTH;
    for (i=0; i < t->size; i++) {
        int tpos = mpos + tmino_squares[i];
        if (tpos >= 0 && tpos < MATRIX_SIZE)
            g->m[tpos].unvisited = false;
    }

    // cut out preview
    if (preview_visible) {
        for (i=0; i<Buffer.len; i++) {
            int x = Buffer.pos[i] % MATRIX_WIDTH + Buffer.x_topleft - Buffer.x_middle;
            int y = Buffer.pos[i] / MATRIX_WIDTH + Buffer.y_topleft - Buffer.y_middle;

            if (x < 0 || x >= MATRIX_WIDTH || y < 0 || y >= MATRIX_HEIGHT)
                continue;
            if (!g->m[x * MATRIX_WIDTH + y].visible)
                g->m[x * MATRIX_WIDTH + y].unvisited = false;
        }
    }


    // draw the whole field on top of the current
    for (i=0; i < MATRIX_SIZE; i++) {
        if (g->m[i].unvisited)
            draw_field(g, i%MATRIX_WIDTH, i/MATRIX_WIDTH);
    }

    // draw the preview
    if (preview_visible)
        draw_preview(g, true);

    // draw the moving piece
    draw_tetramino(g, tmino_squares, 0, t->size);
    call_updaterects(g, 0, 0, MATRIX_WIDTH, MATRIX_HEIGHT);

}

void change_preview_visible(struct game *g, bool visible)
{
    preview_visible = visible;
}

bool mark_valid(struct game *g)
{
    return Mark.len != 0;
}

void move_mark_to_selection(struct game *g)
{
    set_selection(g, false);
    move_buffer(&Mark, &Selection);
    set_selection(g, true);
}

static void move_buffer(blockinfo *bufferFrom, blockinfo *bufferTo)
{
    int i;
    for (i=0; i < bufferFrom->len; i++)
        bufferTo->pos[i] = bufferFrom->pos[i];
    bufferTo->color = bufferFrom->color;
    bufferTo->len = bufferFrom->len;
    bufferTo->x_middle = bufferFrom->x_middle;
    bufferTo->y_middle = bufferFrom->y_middle;
    bufferTo->x_topleft = bufferFrom->x_topleft;
    bufferTo->y_topleft = bufferFrom->y_topleft;
}

static void set_selection(struct game *g, bool selected)
{
    if (Selection.len == 0)
        return;
    // go through the background
    int i;
    for (i=0; i<Selection.len; i++) {
        int x = Selection.pos[i] % MATRIX_WIDTH + Selection.x_topleft;
        int y = Selection.pos[i] / MATRIX_WIDTH + Selection.y_topleft;
        int pos = x + y * MATRIX_WIDTH;
        g->m[pos].selected = selected;
    }
}

static void cut_selection(struct game *g)
{
    if (Selection.len == 0)
        return;
    // go through the background
    int i;
    for (i=0; i<Selection.len; i++) {
        int x = Selection.pos[i] % MATRIX_WIDTH + Selection.x_topleft;
        int y = Selection.pos[i] / MATRIX_WIDTH + Selection.y_topleft;
        int pos = x + y * MATRIX_WIDTH;
        g->m[pos].visible = false;
    }
    Selection.len = 0;
}

bool fits_preview(struct game *g, int x, int y)
{
    return can_pop_tetramino(g, x, y);
}

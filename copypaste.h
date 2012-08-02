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

#ifndef _COPYPASTE_H_
#define _COPYPASTE_H_

#include "tetris.h"

void clean_buffers(struct game *g);
void manage_copy(struct game *g);
void manage_paste(struct game *g);
void manage_cut(struct game *g);

void select_tmino(struct game *g, int x, int y);
//void select_background(struct game *g, int x, int y);
void undraw_preview(struct game *g);
void draw_preview(struct game *g, bool force);
void mark_tmino(struct game *g, int x, int y);
void mark_background(struct game *g, int x, int y);

bool draw_preview_in_pos(struct game *g, int x, int y);

void unmark_background(struct game *g);
void undraw_mark(struct game *g);
void draw_mark(struct game *g);

void redraw_field(struct game *g);
void change_preview_visible(struct game *g, bool visible);
bool mark_valid(struct game *g);
void move_mark_to_selection(struct game *g);
//void unselect_background(struct game *g);
void unselect_tmino(struct game *g);
void unselect_background(struct game *g);


#endif

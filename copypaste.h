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

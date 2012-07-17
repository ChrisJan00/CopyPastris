#ifndef _COPYPASTE_H_
#define _COPYPASTE_H_

#include "tetris.h"

void restart_buffer(struct game *g);
void manage_copy(struct game *g);
void manage_paste(struct game *g);
void manage_cut(struct game *g);

void select_tmino(struct game *g, int x, int y);
void select_background(struct game *g, int x, int y);
void draw_preview(struct game *g);
void mark_tmino(struct game *g, int x, int y);
void mark_background(struct game *g, int x, int y);

#endif

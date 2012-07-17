#ifndef _MOUSE_INPUT_H_
#define _MOUSE_INPUT_H_

#include "tetris.h"

bool get_mousecoords(struct game *g, int *clickedx, int *clickedy);
void mouseHover(struct game *g);
void mouseClicked(struct game *g);

#endif

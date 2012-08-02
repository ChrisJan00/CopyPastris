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

#ifndef _GFX_H_
#define _GFX_H_

#include <SDL/SDL.h>

SDL_Surface *create_surface_wrap(int, int, int, const char *);
#define create_surface(w, h)	create_surface_wrap(w, h, __LINE__, __FILE__)
void fill_gradient(SDL_Surface *);
void draw_border(SDL_Surface *, SDL_Rect, int, Uint32);

#endif

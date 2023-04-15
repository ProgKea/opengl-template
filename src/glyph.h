#ifndef GLYPH_H_
#define GLYPH_H_

#include "renderer.h"

// CODE from tsoding: https://github.com/tsoding/ded
/*
Copyright 2021 Alexey Kutepov <reximkut@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <ft2build.h>
#include FT_FREETYPE_H

#define FREE_GLYPH_FONT_SIZE 100

// https://en.wikibooks.org/wiki/OpenGL_Programming/Modern_OpenGL_Tutorial_Text_Rendering_02

typedef struct {
    float ax; // advance.x
    float ay; // advance.y

    float bw; // bitmap.width;
    float bh; // bitmap.rows;

    float bl; // bitmap_left;
    float bt; // bitmap_top;

    float tx; // x offset of glyph in texture coordinates
} Glyph_Metric;

#define GLYPH_METRICS_CAPACITY 128

typedef struct {
    FT_UInt atlas_width;
    FT_UInt atlas_height;
    GLuint glyphs_texture;
    Glyph_Metric metrics[GLYPH_METRICS_CAPACITY];
} Free_Glyph_Atlas;

void free_glyph_atlas_init(Free_Glyph_Atlas *atlas, FT_Face face);
void free_glyph_atlas_render_line_sized(Free_Glyph_Atlas *atlas, Renderer *r, const char *text, size_t text_size, V2f *pos, V4f color);

#endif  // GLYPH_H_

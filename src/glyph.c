#include <stdlib.h>
#include <stdio.h>
#include "glyph.h"

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
void free_glyph_atlas_init(Free_Glyph_Atlas *atlas, FT_Face face)
{
    FT_Int32 load_flags = FT_LOAD_RENDER | FT_LOAD_TARGET_(FT_RENDER_MODE_SDF);
    for (int i = 32; i < 128; ++i) {
        if (FT_Load_Char(face, i, load_flags)) {
            fprintf(stderr, "ERROR: could not load glyph of character: %d\n", i);
            exit(1);
        }

        atlas->atlas_width += face->glyph->bitmap.width;
        if (atlas->atlas_height < face->glyph->bitmap.rows) {
            atlas->atlas_height = face->glyph->bitmap.rows;
        }
    }

    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &atlas->glyphs_texture);
    glBindTexture(GL_TEXTURE_2D, atlas->glyphs_texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RED,
        (GLsizei) atlas->atlas_width,
        (GLsizei) atlas->atlas_height,
        0,
        GL_RED,
        GL_UNSIGNED_BYTE,
        NULL);

    int x = 0;
    for (int i = 32; i < 128; ++i) {
        if (FT_Load_Char(face, i, load_flags)) {
            fprintf(stderr, "ERROR: could not load glyph of a character: %d\n", i);
            exit(1);
        }

        if (FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL)) {
            fprintf(stderr, "ERROR: could not render glyph of a character: %d\n", i);
            exit(1);
        }

        atlas->metrics[i].ax = face->glyph->advance.x >> 6;
        atlas->metrics[i].ay = face->glyph->advance.y >> 6;
        atlas->metrics[i].bw = face->glyph->bitmap.width;
        atlas->metrics[i].bh = face->glyph->bitmap.rows;
        atlas->metrics[i].bl = face->glyph->bitmap_left;
        atlas->metrics[i].bt = face->glyph->bitmap_top;
        atlas->metrics[i].tx = (float) x / (float) atlas->atlas_width;

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexSubImage2D(GL_TEXTURE_2D,
                        0,
                        x,
                        0,
                        face->glyph->bitmap.width,
                        face->glyph->bitmap.rows,
                        GL_RED,
                        GL_UNSIGNED_BYTE,
                        face->glyph->bitmap.buffer);
        x += face->glyph->bitmap.width;
    }
}

void free_glyph_atlas_render_line_sized(Free_Glyph_Atlas *atlas, Renderer *r, const char *text, size_t text_size, V2f *pos, V4f color)
{
    for (size_t i = 0; i < text_size; ++i) {
        size_t glyph_index = text[i];
        if (glyph_index >= GLYPH_METRICS_CAPACITY) {
            glyph_index = '?';
        }
        Glyph_Metric metric = atlas->metrics[glyph_index];
        float x2 = pos->x + metric.bl;
        float y2 = -pos->y - metric.bt;
        float w  = metric.bw;
        float h  = metric.bh;

        pos->x += metric.ax;
        pos->y += metric.ay;

        renderer_image_rect(r,
                            v2f(x2, -y2),
                            color,
                            v2f(w, -h),
                            v2f(metric.tx, 0),
                            v2f(metric.bw / (float) atlas->atlas_width, metric.bh / (float) atlas->atlas_height));
    }
}

#ifndef RENDERER_H_
#define RENDERER_H_

#define LA_IMPLEMENTATION
#include "la.h"

#include <GL/glew.h>

typedef struct {
    V2f position;
    V4f color;
    V2f uv;
} Vertex;

typedef enum {
    UNIFORM_TIME = 0,
    UNIFORM_RESOLUTION,
    COUNT_UNIFORMS,
} Uniform;

#define VERTICES_CAP (3*5*1024)

typedef struct {
    GLuint vao;
    GLuint vbo;
    GLuint program;

    double time;
    V2f resolution;

    Uniform uniforms[COUNT_UNIFORMS];
    Vertex vertices[VERTICES_CAP];
    size_t vertices_count;
} Renderer;

void renderer_init(Renderer *r); // TODO: Use arena allocator later
void renderer_triangle(Renderer *r,
                       V2f p0, V2f p1, V2f p2,
                       V4f c0, V4f c1, V4f c2,
                       V2f uv0, V2f uv1, V2f uv2);
void renderer_quad(Renderer *r,
                   V2f p0, V2f p1, V2f p2, V2f p3,
                   V4f c0, V4f c1, V4f c2, V4f c3,
                   V2f uv0, V2f uv1, V2f uv2, V2f uv3);
void renderer_rect_gradient(Renderer *r, V2f p0, V4f c0, V4f c1, V4f c2, V4f c3, V2f size);
void renderer_rect_gradient_center(Renderer *r, V2f p0, V4f c0, V4f c1, V4f c2, V4f c3, V2f size);
void renderer_rect(Renderer *r, V2f p0, V4f c0, V2f size);
void renderer_rect_center(Renderer *r, V2f p0, V4f c0, V2f size);
void renderer_flush(Renderer *r);

#endif  // RENDERER_H_

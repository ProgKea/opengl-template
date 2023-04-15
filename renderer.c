#include "renderer.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "common.h"

static Errno read_entire_file(const char *file_path, char **buffer, size_t *buffer_size)
{
    Errno result = 0;
    FILE *f = NULL;

    f = fopen(file_path, "rb");
    if (fseek(f, 0, SEEK_END) < 0) return_defer(errno);
    long m = ftell(f);
    if (m <= 0) return_defer(errno);
    if (fseek(f, 0, SEEK_SET) < 0) return_defer(errno);
    *buffer_size = m;
    *buffer = malloc(*buffer_size+1);
    if (fread(*buffer, *buffer_size, 1, f) != 1) return_defer(errno);
    if (ferror(f) != 0) return_defer(errno);
    (*buffer)[*buffer_size] = '\0';

defer:
    if (f) fclose(f);
    return result;
}

static void read_entire_file_checked(const char *file_path, char **buffer, size_t *buffer_size)
{
    Errno err = read_entire_file(file_path, buffer, buffer_size);
    if (err != 0) {
        fprintf(stderr, "ERROR: Failed to read file %s: %s\n", file_path, strerror(err));
        exit(1);
    }
}

static bool compile_shader_source(GLuint shader, const char *source)
{
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint compiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

    if (!compiled) {
        GLchar message[1024];
        GLsizei message_size = 0;
        glGetShaderInfoLog(shader, sizeof(message), &message_size, message);
        fprintf(stderr, "ERROR: Failed to compile shader\n");
        fprintf(stderr, "%.*s\n", message_size, message);
        return false;
    }

    return true;
}

static bool compile_shader_file(GLuint shader, const char *file_path)
{
    char *content;
    size_t content_size;
    read_entire_file_checked(file_path, &content, &content_size);
    bool result = compile_shader_source(shader, content);
    if (content) free(content);
    return result;
}

static bool link_program(GLuint program)
{
    glLinkProgram(program);

    GLint linked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked) {
        GLchar message[1024];
        GLsizei message_size = 0;
        glGetProgramInfoLog(program, sizeof(message), &message_size, message);
        fprintf(stderr, "%.*s\n", message_size, message);
        return false;
    }

    return true;
}

typedef struct {
    Uniform uniform;
    const char *name;
} Uniform_Def;

static_assert(COUNT_UNIFORMS == 2, "Update definition table for uniforms accordingly");
static const Uniform_Def uniform_defs[COUNT_UNIFORMS] = {
    [UNIFORM_TIME] = {
        .uniform = UNIFORM_TIME,
        .name = "time",
    },
    [UNIFORM_RESOLUTION] = {
        .uniform = UNIFORM_RESOLUTION,
        .name = "resolution",
    },
};

static void get_uniform_locations(GLuint program, Uniform locations[COUNT_UNIFORMS])
{
    for (Uniform u = 0; u < COUNT_UNIFORMS; ++u) {
        locations[u] = glGetUniformLocation(program, uniform_defs[u].name);
    }
}

static void renderer_set_shader(Renderer *r)
{
    glUseProgram(r->program);
    get_uniform_locations(r->program, r->uniforms);
    glUniform2f(r->uniforms[UNIFORM_RESOLUTION], V2f_Arg(r->resolution));
    glUniform1f(r->uniforms[UNIFORM_TIME], (float)r->time);
}

void renderer_init(Renderer *r)
{
    {
        glGenVertexArrays(1, &r->vao);
        glBindVertexArray(r->vao);

        glGenBuffers(1, &r->vbo);
        glBindBuffer(GL_ARRAY_BUFFER, r->vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(r->vertices), r->vertices, GL_DYNAMIC_DRAW);

        // Position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0,
                              2,
                              GL_FLOAT,
                              GL_FALSE,
                              sizeof(Vertex),
                              (GLvoid *) offsetof(Vertex, position));

        // Color
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1,
                              4,
                              GL_FLOAT,
                              GL_FALSE,
                              sizeof(Vertex),
                              (GLvoid *) offsetof(Vertex, color));

        // UV
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2,
                              2,
                              GL_FLOAT,
                              GL_FALSE,
                              sizeof(Vertex),
                              (GLvoid *) offsetof(Vertex, uv));
    }

    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    if (!compile_shader_file(vertex_shader, "shaders/simple.vert")) exit(1);
    if (!compile_shader_file(fragment_shader, "shaders/color.frag")) exit(1);

    r->program = glCreateProgram();
    glAttachShader(r->program, vertex_shader);
    glAttachShader(r->program, fragment_shader);

    if (!link_program(r->program)) exit(1);
    glDetachShader(r->program, vertex_shader);
    glDetachShader(r->program, fragment_shader);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    renderer_set_shader(r);
}

static void renderer_vertex(Renderer *r, V2f p, V4f c, V2f uv)
{
    assert(r->vertices_count < VERTICES_CAP);
    Vertex *last = &r->vertices[r->vertices_count];
    last->position = p;
    last->color    = c;
    last->uv       = uv;
    r->vertices_count += 1;
}

void renderer_triangle(Renderer *r,
                       V2f p0, V2f p1, V2f p2,
                       V4f c0, V4f c1, V4f c2,
                       V2f uv0, V2f uv1, V2f uv2)
{
    renderer_vertex(r, p0, c0, uv0);
    renderer_vertex(r, p1, c1, uv1);
    renderer_vertex(r, p2, c2, uv2);
}

/*
  p2------p3
  | \     |
  |  \    |
  |   \   |
  |    \  |
  |     \ |
  |      \|
  p0------p1
*/

void renderer_quad(Renderer *r,
                   V2f p0, V2f p1, V2f p2, V2f p3,
                   V4f c0, V4f c1, V4f c2, V4f c3,
                   V2f uv0, V2f uv1, V2f uv2, V2f uv3)
{
    renderer_triangle(r, p0, p1, p2, c0, c1, c2, uv0, uv1, uv2);
    renderer_triangle(r, p1, p2, p3, c1, c2, c3, uv1, uv2, uv3);
}

void renderer_rect_gradient(Renderer *r, V2f p0, V4f c0, V4f c1, V4f c2, V4f c3, V2f size)
{
    V2f uv = v2f(0, 0);
    V2f p1 = v2f(p0.x + size.x, p0.y);
    V2f p2 = v2f(p0.x, p0.y + size.y);
    V2f p3 = v2f(p0.x + size.x, p0.y + size.y);
    renderer_quad(r, p0, p1, p2, p3, c0, c1, c2, c3, uv, uv, uv, uv);
}

void renderer_rect_gradient_center(Renderer *r, V2f p0, V4f c0, V4f c1, V4f c2, V4f c3, V2f size)
{
    renderer_rect_gradient(r, v2f(p0.x - size.x/2, p0.y - size.y/2), c0, c1, c2, c3, size);
}

void renderer_rect(Renderer *r, V2f p0, V4f c0, V2f size)
{
    renderer_rect_gradient(r, p0, c0, c0, c0, c0, size);
}

void renderer_rect_center(Renderer *r, V2f p0, V4f c0, V2f size)
{
    renderer_rect(r, v2f(p0.x - size.x/2, p0.y - size.y/2), c0, size);
}

static void renderer_sync(Renderer *r)
{
    glBufferSubData(GL_ARRAY_BUFFER,
                    0,
                    sizeof(Vertex) * r->vertices_count,
                    r->vertices);
}

static void renderer_draw(Renderer *r)
{
    glDrawArrays(GL_TRIANGLES, 0, r->vertices_count);
}

void renderer_flush(Renderer *r)
{
    renderer_sync(r);
    renderer_draw(r);
    renderer_set_shader(r);
    r->vertices_count = 0;
}

void renderer_deinit(Renderer *r)
{
    free(r);
}

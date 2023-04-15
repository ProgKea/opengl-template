#include "renderer.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "common.h"

#define vert_shader_file_path "./shaders/simple.vert"

static_assert(COUNT_SHADERS == 3, "The amount of fragment shaders has changed");
const char *frag_shader_file_paths[COUNT_SHADERS] = {
    [SHADER_COLOR] = "./shaders/color.frag",
    [SHADER_TEXT] = "./shaders/text.frag",
    [SHADER_RAINBOW] = "./shaders/rainbow.frag",
};

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

static bool compile_shader_source(GLuint *shader, GLenum shader_type, const char *source)
{
    *shader = glCreateShader(shader_type);
    glShaderSource(*shader, 1, &source, NULL);
    glCompileShader(*shader);

    GLint compiled = 0;
    glGetShaderiv(*shader, GL_COMPILE_STATUS, &compiled);

    if (!compiled) {
        GLchar message[1024];
        GLsizei message_size = 0;
        glGetShaderInfoLog(*shader, sizeof(message), &message_size, message);
        fprintf(stderr, "ERROR: Failed to compile shader\n");
        fprintf(stderr, "%.*s\n", message_size, message);
        return false;
    }

    return true;
}

static bool compile_shader_file(GLuint *shader, GLenum shader_type, const char *file_path)
{
    char *content;
    size_t content_size;
    read_entire_file_checked(file_path, &content, &content_size);
    bool result = compile_shader_source(shader, shader_type, content);
    if (content) free(content);
    return result;
}

static void attach_shaders_to_program(GLuint *shaders, size_t shaders_count, GLuint program)
{
    for (size_t i = 0; i < shaders_count; ++i) {
        glAttachShader(program, shaders[i]);
    }
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

    // GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    // GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    // if (!compile_shader_file(vertex_shader, "shaders/simple.vert")) exit(1);
    // if (!compile_shader_file(fragment_shader, "shaders/text.frag")) exit(1);

    // r->program = glCreateProgram();
    // glAttachShader(r->program, vertex_shader);
    // glAttachShader(r->program, fragment_shader);

    // if (!link_program(r->program)) exit(1);
    // glDetachShader(r->program, vertex_shader);
    // glDetachShader(r->program, fragment_shader);
    // glDeleteShader(vertex_shader);
    // glDeleteShader(fragment_shader);

    // renderer_set_shader(r);

    GLuint shaders[2] = {0};

    if (!compile_shader_file(&shaders[0], GL_VERTEX_SHADER, vert_shader_file_path)) exit(1);

    for (int i = 0; i < COUNT_SHADERS; ++i) {
        if (!compile_shader_file(&shaders[1], GL_FRAGMENT_SHADER, frag_shader_file_paths[i])) exit(1);
        r->programs[i] = glCreateProgram();
        attach_shaders_to_program(shaders, sizeof(shaders) / sizeof(shaders[0]), r->programs[i]);
        if (!link_program(r->programs[i])) exit(1);
        glDetachShader(r->programs[i], shaders[1]);
        glDetachShader(r->programs[i], shaders[0]);
        glDeleteShader(shaders[1]);
    }
    glDeleteShader(shaders[0]);
}

void renderer_set_shader(Renderer *r, Shader shader)
{
    r->current_shader = shader;
    glUseProgram(r->programs[r->current_shader]);
    get_uniform_locations(r->programs[r->current_shader], r->uniforms);
    glUniform2f(r->uniforms[UNIFORM_RESOLUTION], V2f_Arg(r->resolution));
    glUniform1f(r->uniforms[UNIFORM_TIME], (float)r->time);
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

void renderer_image_rect(Renderer *r, V2f p0, V4f c0, V2f size, V2f uvp, V2f uvs)
{
    renderer_quad(r,
                  p0, v2f_sum(p0, v2f(size.x, 0)), v2f_sum(p0, v2f(0, size.y)), v2f_sum(p0, size),
                  c0, c0, c0, c0,
                  uvp, v2f_sum(uvp, v2f(uvs.x, 0)), v2f_sum(uvp, v2f(0, uvs.y)), v2f_sum(uvp, uvs));
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
    r->vertices_count = 0;
}

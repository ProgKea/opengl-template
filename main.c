#include <stdio.h>

#include <GL/glew.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "common.h"
#include "renderer.h"

static void debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam)
{
    (void) source;
    (void) type;
    (void) id;
    (void) severity;
    (void) length;
    (void) userParam;
    fprintf(stderr, "ERROR: %s\n", message);
}

static void glfw_error_callback(int error, const char *desc)
{
    (void) error;
    fprintf(stderr, "ERROR: %s\n", desc);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    (void) scancode;
    (void) mods;
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

static Renderer renderer = {0};

int main()
{
    int result = 0;

    glfwSetErrorCallback(glfw_error_callback);

    GLFWwindow *window = NULL;
    glewExperimental = GL_TRUE;
    if (!glfwInit()) {
        return_defer(1);
    }

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // Doesn't work for some reason
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "App", 0, 0);
    if (!window) {
        return_defer(1);
    }
    glfwMakeContextCurrent(window);

    GLenum glew_err = glewInit();
    if (glew_err != GLEW_OK) {
        fprintf(stderr, "ERROR: %s\n", glewGetErrorString(glew_err));
        return_defer(1);
    }
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(debug_callback, NULL);
    printf("GLEW   version: %s\n", glewGetString(GLEW_VERSION));
    printf("OpenGL version: %s\n", glGetString(GL_VERSION));

    renderer_init(&renderer);

    V2f rect_pos  = v2f(SCREEN_WIDTH/2, SCREEN_HEIGHT/2);
    V2f rect_vel  = v2f(1, 1);
    V2f rect_size = v2f(100, 100);
    float rect_speed = 1;

    glClearColor(1, 1, 1, 1);
    glfwSetKeyCallback(window, key_callback);
    glfwSwapInterval(1);
    while (!glfwWindowShouldClose(window)) {
        renderer.time = glfwGetTime();

        int cur_width, cur_height;
        glfwGetFramebufferSize(window, &cur_width, &cur_height);
        renderer.resolution = v2f(cur_width, cur_height);
        glViewport(0, 0, cur_width, cur_height);
        glClear(GL_COLOR_BUFFER_BIT);

        renderer_rect_center(&renderer, rect_pos, v4f(0, 0, 0, 1), rect_size);

        rect_pos = v2f_sum(rect_pos, v2f(rect_speed * rect_vel.x, rect_speed * rect_vel.y));
        if (rect_pos.x + rect_size.x/2 >= SCREEN_WIDTH) rect_vel = v2f_mul(rect_vel, v2f(-1, 1));
        if (rect_pos.y + rect_size.y/2 >= SCREEN_HEIGHT) rect_vel = v2f_mul(rect_vel, v2f(1, -1));
        if (rect_pos.x - rect_size.x/2 <= 0) rect_vel = v2f_mul(rect_vel, v2f(-1, 1));
        if (rect_pos.y - rect_size.y/2 <= 0) rect_vel = v2f_mul(rect_vel, v2f(1, -1));

        renderer_flush(&renderer);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

defer:
    if (window) glfwDestroyWindow(window);
    return result;
}

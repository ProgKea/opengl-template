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

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Kanview", 0, 0);
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

        renderer_rect_gradient(&renderer, v2f(0, 0),
                               v4f(1, 1, 0, 1), v4f(1, 1, 0, 1), v4f(1, 1, 1, 1), v4f(1, 1, 1, 1),
                               v2f(cur_width, cur_height/2));
        renderer_rect_gradient(&renderer, v2f(0, cur_height/2),
                               v4f(1, 1, 1, 1), v4f(1, 1, 1, 1), v4f(1, 1, 0, 1), v4f(1, 1, 0, 1),
                               v2f(cur_width, cur_height/2));

        renderer_flush(&renderer);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

defer:
    if (window) glfwDestroyWindow(window);
    return result;
}

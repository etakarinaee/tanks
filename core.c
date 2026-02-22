
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <stdio.h>

#include <luajit-2.1/lua.h>

#include <core_archive.h>
#include <core_lua.h>

#include <core_renderer.h>

/* where the game data is stored */
#define SAUSAGES_DATA "sausages.arc"
/* entrypoint in lua */
#define SAUSAGES_ENTRY "game.lua"

static lua_State *L;

void resize_callback(GLFWwindow* window, int width, int height) {
    struct render_context* ctx = (struct render_context*)glfwGetWindowUserPointer(window);
    ctx->width = width;
    ctx->height = height;
    glViewport(0, 0, width, height);
}

int main() {
    GLFWwindow* window;
    FILE* test;
    struct render_context ctx;
    double curr_time, last_time;
    double delta_time;

    /* check for game data before doing anything */
    test = fopen(SAUSAGES_DATA, "rb");
    if (!test) {
        fprintf(stderr, "game data not available\n");
        return 1;
    }
    fclose(test);

    if (!glfwInit()) {
        fprintf(stderr, "failed to init glfw\n");
        return EXIT_FAILURE;
    }

    window = glfwCreateWindow(1200, 800, "Sausages", NULL, NULL);
    if (!window) {
        fprintf(stderr, "failed to create window\n");
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(window);
    gladLoadGL();

    printf("OpenGL %s\n", glGetString(GL_VERSION));

    glfwSetFramebufferSizeCallback(window, resize_callback);
    glfwSetWindowUserPointer(window, &ctx);

    /* Init Lua */
    L = lua_init(SAUSAGES_DATA, SAUSAGES_ENTRY);
    if (!L) {
        fprintf(stderr, "lua_init()\n");

        return 1;
    }
    lua_call_init(L);

    renderer_init(&ctx);


    while (!glfwWindowShouldClose(window)) {
        delta_time = curr_time - last_time;
        last_time = curr_time;
        curr_time = glfwGetTime();

        L = lua_reload(L, SAUSAGES_DATA, SAUSAGES_ENTRY);
        lua_call_update(L, delta_time);

        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.2f, 0.3f, 0.4f, 1.0f);

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            double x; /* Not sure about the c89 std in this next lines here */
            double y;
            glfwGetCursorPos(window, &x, &y);
            struct vec2 pos;

            /* convert mouse pos from window coords to 0 to 1 by dividing with width 
                than convert to -1 to 1 by multiply 2 and then subtract -1
            */
            pos = (struct vec2){ (x / ctx.width) * 2.0f - 1.0f, -((y / ctx.height) * 2.0f - 1.0f) };

            /* printf("X: %f Y: %f\n", pos.x, pos.y); */

            renderer_push_quad(&ctx, pos, 1.0f, 0.0f);
        }

        renderer_draw(&ctx);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    renderer_deinit(&ctx);
    glfwTerminate();

    lua_quit(L);

    return 0;
}



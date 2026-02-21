
#ifndef REDNERER_H
#define REDNERER_H

#include <glad/glad.h>

struct vec2 {
    float x;
    float y;
};

struct quad_data {
    struct vec2 pos;
    float scale;
    float rotation;
};

struct render_context {
    int width;
    int height;

    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    GLuint program;
};

int renderer_init(struct render_context* ctx);
void renderer_push_quad(struct render_context* ctx, struct vec2, float scale, float rotation);

#endif 

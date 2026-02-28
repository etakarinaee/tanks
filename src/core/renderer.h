#ifndef RENDERER_H
#define RENDERER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <freetype2/ft2build.h>
#include FT_FREETYPE_H

#include <stddef.h>
#include <math.h>

#include "cmath.h"

#define CORE_RENDERER_QUAD_NO_TEXTURE (-1)

struct color3 {
    float r;
    float g;
    float b;
};

typedef GLint texture_id;
typedef int font_id;

struct quad_data {
    texture_id tex;
    float scale;
    float rotation;
    struct vec2 pos;
    struct color3 color;
};

struct character {
    struct vec2i size;
    struct vec2i bearing;
    uint32_t advance;
};

struct font {
    struct vec2i char_range; /* from this asci code to another all chars its tmp for more advanced loading */ 
    struct character* chars; /* character data array as big as char_range.y - char_range.x */
    texture_id tex;
};

struct camera {
    struct vec2 pos;
    float zoom;
};

struct render_context {
    int width;
    int height;
    GLFWwindow *window;
    struct camera camera;

    /* Text */
    FT_Library ft_lib;
    struct font* fonts;
    size_t fonts_count;
    size_t fonts_capacity;

    GLuint vao;
    GLuint vbo;
    GLuint ebo;

    GLuint quad_program;
    GLuint tex_program;

    struct quad_data *quads;
    size_t quads_count;
    size_t quads_capacity;
};

extern struct render_context ctx;

int renderer_init(struct render_context *r);
void renderer_deinit(const struct render_context *r);
void renderer_push_quad(struct render_context *r, struct vec2 pos, float scale, float rotation, struct color3 color, texture_id tex);
void renderer_draw(struct render_context *r);

texture_id renderer_load_texture(const char *path);
texture_id renderer_load_font(struct render_context *r, const char* path);

#endif

#include "renderer.h"
#include "archive.h"
#include "font.h"
#include "freetype/freetype.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stbi/stb_image.h>

struct render_context ctx;

static const float rectangle_vertices[] = {
    /* POS           UV     */     
    0.5f, 0.5f,     1.0f, 1.0f,  
    0.5f, -0.5f,    1.0f, 0.0f,  
    -0.5f, -0.5f,   0.0f, 0.0f,  
    -0.5f, 0.5f,    0.0f, 1.0f,
};

static const unsigned int rectangle_indices[] = {
    0, 1, 3,
    1, 2, 3
};

static void buffers_init(struct render_context *r) {
    glGenVertexArrays(1, &r->vao);
    glBindVertexArray(r->vao);

    glGenBuffers(1, &r->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, r->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rectangle_vertices), rectangle_vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &r->ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(rectangle_indices), rectangle_indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

static int program_init(const char *vert_path, const char *frag_path, GLuint *program) {
    int error = 0;
    int success;
    char info_log[512];
    uint32_t len;

    char *vertex_str = NULL;
    char *fragment_str = NULL;
    GLuint vertex_id = 0, fragment_id = 0;

    /* Vertex Shader */
    vertex_str = archive_read_alloc(SAUSAGES_DATA, vert_path, &len);
    if (!vertex_str) {
        error = 1;
        goto end;
    }

    vertex_id = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_id, 1, (const GLchar *const *)&vertex_str, NULL);
    glCompileShader(vertex_id);

    glGetShaderiv(vertex_id, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex_id, 512, NULL, info_log);
        fprintf(stderr, "Vertex Shader Compilation Failed: %s!\n", info_log);
        error = 1;
        goto end;
    }

    /* Fragment Shader */
    fragment_str = archive_read_alloc(SAUSAGES_DATA, frag_path, &len);
    if (!fragment_str) {
        error = 1;
        goto end;
    }

    fragment_id = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_id, 1, (const GLchar *const *)&fragment_str, NULL);
    glCompileShader(fragment_id);

    glGetShaderiv(fragment_id, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment_id, 512, NULL, info_log);
        fprintf(stderr, "Fragment Shader Compilation Failed: %s!\n", info_log);
        error = 1;
        goto end;
    }

    *program = glCreateProgram();
    glAttachShader(*program, vertex_id);
    glAttachShader(*program, fragment_id);
    glLinkProgram(*program);

    glGetProgramiv(*program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(*program, 512, NULL, info_log);
        fprintf(stderr, "failed to link shaders: %s\n", info_log);
        error = 1;
        goto end;
    }

    glUseProgram(*program);

end:
    glDeleteShader(vertex_id);
    glDeleteShader(fragment_id);
    free(vertex_str);
    free(fragment_str);

    return error;
}

int renderer_init(struct render_context *r) {
    buffers_init(r);

    if (program_init("quad.vert", "quad.frag", &r->quad_program)) {
        fprintf(stderr, "failed to init shaders!\n");
        return 1;
    }

    if (program_init("texture.vert", "texture.frag", &r->tex_program)) {
        fprintf(stderr, "failed to init shaders!\n");
        return 1;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    r->camera.pos = (struct vec2){0.0f, 0.0f};
    r->camera.zoom = 1.0f;

    r->quads = calloc(2, sizeof(struct quad_data));
    r->quads_count = 0;
    r->quads_capacity = 2;

    if (font_init(r)) {
        fprintf(stderr, "failed initing font engine\n");
        return 1;
    }

    return 0;
}

void renderer_deinit(const struct render_context *r) {
    free(r->quads);
    font_deinit(r);
}

void renderer_push_quad(struct render_context *r, const struct vec2 pos, const float scale, const float rotation, const struct color3 color, const texture_id tex) {
    if (r->quads_count + 1 > r->quads_capacity) {
        const size_t new_cap = r->quads_capacity ? r->quads_capacity * 2 : 2;
        struct quad_data *new_data = realloc(r->quads, new_cap * sizeof(struct quad_data));
        if (!new_data) {
            fprintf(stderr, "out of memory!\n");
            return;
        }
        r->quads = new_data;
        r->quads_capacity = new_cap;
    }

    r->quads[r->quads_count++] = (struct quad_data){
        .pos = pos,
        .scale = scale,
        .rotation = rotation,
        .color = color,
        .tex = tex
    };
}

void renderer_draw(struct render_context *r) {
    glBindVertexArray(r->vao);

    struct matrix cam_m;
    math_matrix_get_orthographic(r, &cam_m);

    for (size_t i = 0; i < r->quads_count; i++) {
        const struct quad_data *data = &r->quads[i];

        /* TODO: Make this clean */
        struct matrix translate_m;
        math_matrix_translate(&translate_m, data->pos.x, data->pos.y, 0.0f);

        struct matrix scale_m;
        math_matrix_scale(&scale_m, data->scale, data->scale, data->scale);

        struct matrix rotate_m;
        math_matrix_rotate_2d(&rotate_m, data->rotation);

        struct matrix scale_rot_m;
        math_matrix_mul(&scale_rot_m, &scale_m, &rotate_m);

        struct matrix m;
        math_matrix_mul(&m, &translate_m, &scale_rot_m);

        GLint uniform_matrix_model_loc;
        GLint uniform_matrix_cam_loc;
        if (data->tex == CORE_RENDERER_QUAD_NO_TEXTURE) {
            glUseProgram(r->quad_program);
            uniform_matrix_model_loc = glGetUniformLocation(r->quad_program, "u_model");
            uniform_matrix_cam_loc = glGetUniformLocation(r->quad_program, "u_proj");

            const GLint uniform_color_loc = glGetUniformLocation(r->quad_program, "u_color");
            glUniform3f(uniform_color_loc, data->color.r, data->color.g, data->color.b);

        } else {
            glUseProgram(r->tex_program);
            uniform_matrix_model_loc = glGetUniformLocation(r->tex_program, "u_model");
            uniform_matrix_cam_loc = glGetUniformLocation(r->tex_program, "u_proj");

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, data->tex);
            GLint sampler_loc = glGetUniformLocation(r->tex_program, "u_texture");
            glUniform1i(sampler_loc, 0);
        }

        glUniformMatrix4fv(uniform_matrix_model_loc, 1, GL_FALSE, m.m);
        glUniformMatrix4fv(uniform_matrix_cam_loc, 1, GL_FALSE, cam_m.m);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

    r->quads_count = 0;
}

texture_id renderer_load_texture(const char *path) {
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_set_flip_vertically_on_load(1);

    int width, height, channels;
    unsigned char *data = stbi_load(path, &width, &height, &channels, 0);
    if (!data) {
        fprintf(stderr, "failed to load texture: %s\n", path);
        return -1;
    }

    const GLint format = channels == 4 ? GL_RGBA : channels == 1 ? GL_RED : GL_RGB;

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);

    return (texture_id)texture;
}

font_id renderer_load_font(const char *path) {
    int width, height;
    uint8_t* data = font_get_atlas(path, &width, &height);

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    /* TODO: idk implement.. */
}


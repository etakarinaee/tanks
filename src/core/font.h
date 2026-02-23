
#ifndef FONT_H
#define FONT_H 

#include <freetype2/ft2build.h>
#include FT_FREETYPE_H

#include <stdint.h>

struct render_context;

struct edge {
    int type;
};

int font_init(struct render_context* r);
void font_deinit(const struct render_context* r);

uint8_t* font_get_atlas(const char* path, int *width, int *height);

#endif // FONT_H

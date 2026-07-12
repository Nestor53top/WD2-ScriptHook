#pragma once

#include "core.h"

typedef struct _WD2_COLOR {
    float r, g, b, a;
} WD2_COLOR;

typedef struct _WD2_RECT {
    float x, y, w, h;
} WD2_RECT;

typedef struct _WD2_POINT {
    float x, y;
} WD2_POINT;

BOOL wd2_render_init(IDXGISwapChain *pSwapChain);
void wd2_render_shutdown(void);
void wd2_render_begin_frame(void);
void wd2_render_end_frame(void);
void wd2_render_draw_text(float x, float y, const char *text, float r, float g, float b, float a, float size);
void wd2_render_draw_rect(float x, float y, float w, float h, float r, float g, float b, float a);
void wd2_render_draw_line(float x1, float y1, float x2, float y2, float r, float g, float b, float a, float thickness);
void wd2_render_draw_circle(float cx, float cy, float radius, float r, float g, float b, float a);
void wd2_render_on_resize(UINT width, UINT height);

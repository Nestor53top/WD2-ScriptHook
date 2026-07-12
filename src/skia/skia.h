#pragma once
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>

// Opaque render context
typedef struct WD2_RenderContext WD2_RenderContext;

// Color type
typedef struct { float r, g, b, a; } SkColor;
static SkColor SkRGBA(float r, float g, float b, float a) { return (SkColor){r,g,b,a}; }

// Rectangle type
typedef struct { float x, y, w, h; } SkRect;

// Surface management
WD2_RenderContext* skia_CreateRenderContext(ID3D11Device* dev, ID3D11DeviceContext* ctx, IDXGISwapChain* sc);
void skia_DestroyRenderContext(WD2_RenderContext* ctx);
void skia_OnResize(WD2_RenderContext* ctx, int w, int h);

// Frame management
void skia_BeginFrame(WD2_RenderContext* ctx);
void skia_EndFrame(WD2_RenderContext* ctx);

// Drawing primitives
void skia_DrawText(WD2_RenderContext* ctx, float x, float y, const char* text, SkColor color, float size);
void skia_DrawRect(WD2_RenderContext* ctx, SkRect rect, SkColor color);
void skia_DrawRectFilled(WD2_RenderContext* ctx, SkRect rect, SkColor color);
void skia_DrawLine(WD2_RenderContext* ctx, float x1, float y1, float x2, float y2, SkColor color, float thickness);
void skia_DrawCircle(WD2_RenderContext* ctx, float cx, float cy, float radius, SkColor color, float thickness);
void skia_DrawCircleFilled(WD2_RenderContext* ctx, float cx, float cy, float radius, SkColor color);
void skia_DrawTriangle(WD2_RenderContext* ctx, float x1, float y1, float x2, float y2, float x3, float y3, SkColor color);
void skia_SetClipRect(WD2_RenderContext* ctx, SkRect rect);
void skia_ClearClip(WD2_RenderContext* ctx);

// Font management
int skia_LoadFont(WD2_RenderContext* ctx, const char* path, float size);
void skia_SetFont(WD2_RenderContext* ctx, int fontId);
void skia_MeasureText(WD2_RenderContext* ctx, const char* text, float size, float* outW, float* outH);

// Image management
int skia_LoadImage(WD2_RenderContext* ctx, const char* path);
void skia_DrawImage(WD2_RenderContext* ctx, int imageId, float x, float y, float w, float h, float rotation, SkColor tint);
void skia_DrawImageRegion(WD2_RenderContext* ctx, int imageId, SkRect src, SkRect dst, float rotation, SkColor tint);

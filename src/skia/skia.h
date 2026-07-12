#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FONTS 16
#define GLYPH_CACHE_SIZE 256
#define ATLAS_WIDTH 1024
#define ATLAS_HEIGHT 1024
#define MAX_IMAGES 64

typedef struct { float r, g, b, a; } SkColor;
static SkColor SkRGBA(float r, float g, float b, float a) { return (SkColor){r,g,b,a}; }
typedef struct { float x, y, w, h; } SkRect;

typedef struct {
    float u0, v0, u1, v1;
    int width, height;
    int advanceX;
} GlyphInfo;

typedef struct {
    int id;
    char path[260];
    float size;
    int textureWidth, textureHeight;
    unsigned char* atlasPixels;
    ID3D11Texture2D* atlasTexture;
    ID3D11ShaderResourceView* atlasSRV;
    GlyphInfo glyphs[GLYPH_CACHE_SIZE];
    int glyphCount;
} SkFontEntry;

typedef struct {
    int id;
    int width, height;
    ID3D11Texture2D* texture;
    ID3D11ShaderResourceView* srv;
} SkImageEntry;

typedef struct {
    ID3D11VertexShader* vertexShader;
    ID3D11PixelShader* pixelShader;
    ID3D11InputLayout* inputLayout;
    ID3D11Buffer* vertexBuffer;
    ID3D11Buffer* indexBuffer;
    ID3D11Buffer* constantBuffer;
    ID3D11BlendState* blendState;
    ID3D11RasterizerState* rasterizerState;
    ID3D11DepthStencilState* depthStencilState;
    ID3D11SamplerState* samplerState;
    ID3D11Texture2D* fontTexture;
    ID3D11ShaderResourceView* fontTextureSRV;
    ID3D11RenderTargetView* renderTargetView;
    ID3D11Device* device;
    ID3D11DeviceContext* context;
    IDXGISwapChain* swapChain;
    int width, height;
    int vertexCount, indexCount;
    int vertexBufferCapacity;
    SkFontEntry fonts[MAX_FONTS];
    int fontCount;
    int activeFont;
    SkImageEntry images[MAX_IMAGES];
    int imageCount;
} WD2_RenderContext;

WD2_RenderContext* skia_CreateRenderContext(ID3D11Device* dev, ID3D11DeviceContext* ctx, IDXGISwapChain* sc);
void skia_DestroyRenderContext(WD2_RenderContext* ctx);
void skia_OnResize(WD2_RenderContext* ctx, int w, int h);
void skia_BeginFrame(WD2_RenderContext* ctx);
void skia_EndFrame(WD2_RenderContext* ctx);

void skia_DrawText(WD2_RenderContext* ctx, float x, float y, const char* text, SkColor color, float size);
void skia_DrawRect(WD2_RenderContext* ctx, SkRect rect, SkColor color);
void skia_DrawRectFilled(WD2_RenderContext* ctx, SkRect rect, SkColor color);
void skia_DrawLine(WD2_RenderContext* ctx, float x1, float y1, float x2, float y2, SkColor color, float thickness);
void skia_DrawCircle(WD2_RenderContext* ctx, float cx, float cy, float radius, SkColor color, float thickness);
void skia_DrawCircleFilled(WD2_RenderContext* ctx, float cx, float cy, float radius, SkColor color);
void skia_DrawTriangle(WD2_RenderContext* ctx, float x1, float y1, float x2, float y2, float x3, float y3, SkColor color);
void skia_SetClipRect(WD2_RenderContext* ctx, SkRect rect);
void skia_ClearClip(WD2_RenderContext* ctx);

int skia_LoadFont(WD2_RenderContext* ctx, const char* path, float size);
void skia_SetFont(WD2_RenderContext* ctx, int fontId);
void skia_MeasureText(WD2_RenderContext* ctx, const char* text, float size, float* outW, float* outH);

int skia_LoadImage(WD2_RenderContext* ctx, const char* path);
void skia_DrawImage(WD2_RenderContext* ctx, int imageId, float x, float y, float w, float h, float rotation, SkColor tint);
void skia_DrawImageRegion(WD2_RenderContext* ctx, int imageId, SkRect src, SkRect dst, float rotation, SkColor tint);

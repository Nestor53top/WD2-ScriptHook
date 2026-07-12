#include "skia.h"
#include <stdlib.h>
#include <string.h>

#define MAX_FONTS 16
#define GLYPH_CACHE_SIZE 256
#define ATLAS_WIDTH 1024
#define ATLAS_HEIGHT 1024

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
} FontEntry;

struct WD2_FontContext {
    FontEntry fonts[MAX_FONTS];
    int fontCount;
    int activeFont;
    ID3D11Device* device;
    ID3D11DeviceContext* context;
};

static int stbtt_InitFont(void* info, const unsigned char* data, int offset) {
    (void)info; (void)data; (void)offset;
    return 0;
}

static void CreateFontTexture(WD2_FontContext* fctx, FontEntry* font) {
    if (!fctx->device) return;
    D3D11_TEXTURE2D_DESC desc = {0};
    desc.Width = ATLAS_WIDTH;
    desc.Height = ATLAS_HEIGHT;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    D3D11_SUBRESOURCE_DATA initData = {0};
    initData.pSysMem = font->atlasPixels;
    initData.SysMemPitch = ATLAS_WIDTH;
    fctx->device->lpVtbl->CreateTexture2D(fctx->device, &desc, &initData, &font->atlasTexture);
    fctx->device->lpVtbl->CreateShaderResourceView(fctx->device, (ID3D11Resource*)font->atlasTexture, NULL, &font->atlasSRV);
}

int skia_LoadFont(WD2_RenderContext* ctx, const char* path, float size) {
    if (!ctx || !path) return -1;
    WD2_FontContext* fctx = (WD2_FontContext*)ctx;
    if (fctx->fontCount >= MAX_FONTS) return -1;

    FontEntry* font = &fctx->fonts[fctx->fontCount];
    memset(font, 0, sizeof(FontEntry));
    font->id = fctx->fontCount;
    strncpy(font->path, path, sizeof(font->path) - 1);
    font->size = size;
    font->atlasPixels = (unsigned char*)calloc(ATLAS_WIDTH * ATLAS_HEIGHT, 1);
    font->glyphCount = 0;

    /* Load TrueType font file */
    /* Generate atlas with stb_truetype or similar */
    CreateFontTexture(fctx, font);

    return fctx->fontCount++;
}

void skia_SetFont(WD2_RenderContext* ctx, int fontId) {
    if (!ctx) return;
    WD2_FontContext* fctx = (WD2_FontContext*)ctx;
    if (fontId >= 0 && fontId < fctx->fontCount) {
        fctx->activeFont = fontId;
    }
}

void skia_MeasureText(WD2_RenderContext* ctx, const char* text, float size, float* outW, float* outH) {
    if (!ctx || !text) {
        if (outW) *outW = 0;
        if (outH) *outH = 0;
        return;
    }
    WD2_FontContext* fctx = (WD2_FontContext*)ctx;
    FontEntry* font = &fctx->fonts[fctx->activeFont];
    float totalW = 0;
    float maxH = size;
    while (*text) {
        unsigned char c = (unsigned char)*text;
        if (c < GLYPH_CACHE_SIZE && font->glyphs[c].width > 0) {
            totalW += font->glyphs[c].advanceX;
        } else {
            totalW += size * 0.6f;
        }
        text++;
    }
    if (outW) *outW = totalW;
    if (outH) *outH = maxH;
}

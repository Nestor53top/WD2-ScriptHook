#include "skia.h"

static int stbtt_InitFont(void* info, const unsigned char* data, int offset) {
    (void)info; (void)data; (void)offset;
    return 0;
}

static void CreateFontTexture(WD2_RenderContext* rctx, SkFontEntry* font) {
    if (!rctx->device) return;
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
    rctx->device->lpVtbl->CreateTexture2D(rctx->device, &desc, &initData, &font->atlasTexture);
    rctx->device->lpVtbl->CreateShaderResourceView(rctx->device, (ID3D11Resource*)font->atlasTexture, NULL, &font->atlasSRV);
}

int skia_LoadFont(WD2_RenderContext* ctx, const char* path, float size) {
    if (!ctx || !path) return -1;
    if (ctx->fontCount >= MAX_FONTS) return -1;

    SkFontEntry* font = &ctx->fonts[ctx->fontCount];
    memset(font, 0, sizeof(SkFontEntry));
    font->id = ctx->fontCount;
    strncpy(font->path, path, sizeof(font->path) - 1);
    font->size = size;
    font->atlasPixels = (unsigned char*)calloc(ATLAS_WIDTH * ATLAS_HEIGHT, 1);
    font->glyphCount = 0;

    CreateFontTexture(ctx, font);

    return ctx->fontCount++;
}

void skia_SetFont(WD2_RenderContext* ctx, int fontId) {
    if (!ctx) return;
    if (fontId >= 0 && fontId < ctx->fontCount) {
        ctx->activeFont = fontId;
    }
}

void skia_MeasureText(WD2_RenderContext* ctx, const char* text, float size, float* outW, float* outH) {
    if (!ctx || !text) {
        if (outW) *outW = 0;
        if (outH) *outH = 0;
        return;
    }
    SkFontEntry* font = &ctx->fonts[ctx->activeFont];
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

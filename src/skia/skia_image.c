#include "skia.h"

static int stbi_load_from_memory(const unsigned char* buffer, int len, int* x, int* y, int* comp, int req_comp) {
    (void)buffer; (void)len; (void)x; (void)y; (void)comp; (void)req_comp;
    return 0;
}

static void* stbi_image_free(void* retval_stbi_free, int w, int h, int comp) {
    (void)w; (void)h; (void)comp;
    free(retval_stbi_free);
    return NULL;
}

static unsigned char* LoadPNGFile(const char* path, int* outW, int* outH) {
    FILE* f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    unsigned char* data = (unsigned char*)malloc(size);
    if (!data) { fclose(f); return NULL; }
    fread(data, 1, size, f);
    fclose(f);
    int w = 0, h = 0, comp = 0;
    unsigned char* pixels = NULL;
    /* stbi_load_from_memory(data, size, &w, &h, &comp, 4); */
    free(data);
    if (outW) *outW = w;
    if (outH) *outH = h;
    return pixels;
}

static void CreateImageTexture(WD2_RenderContext* rctx, SkImageEntry* img, unsigned char* pixels) {
    if (!rctx->device || !pixels) return;
    D3D11_TEXTURE2D_DESC desc = {0};
    desc.Width = img->width;
    desc.Height = img->height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    D3D11_SUBRESOURCE_DATA initData = {0};
    initData.pSysMem = pixels;
    initData.SysMemPitch = img->width * 4;
    rctx->device->lpVtbl->CreateTexture2D(rctx->device, &desc, &initData, &img->texture);
    rctx->device->lpVtbl->CreateShaderResourceView(rctx->device, (ID3D11Resource*)img->texture, NULL, &img->srv);
}

int skia_LoadImage(WD2_RenderContext* ctx, const char* path) {
    if (!ctx || !path) return -1;
    if (ctx->imageCount >= MAX_IMAGES) return -1;

    int w, h;
    unsigned char* pixels = LoadPNGFile(path, &w, &h);
    if (!pixels) return -1;

    SkImageEntry* img = &ctx->images[ctx->imageCount];
    memset(img, 0, sizeof(SkImageEntry));
    img->id = ctx->imageCount;
    img->width = w;
    img->height = h;
    CreateImageTexture(ctx, img, pixels);
    free(pixels);

    return ctx->imageCount++;
}

void skia_DrawImage(WD2_RenderContext* ctx, int imageId, float x, float y, float w, float h, float rotation, SkColor tint) {
    if (!ctx) return;
    if (imageId < 0 || imageId >= ctx->imageCount) return;
    SkImageEntry* img = &ctx->images[imageId];
    if (!img->srv) return;
    (void)rotation; (void)tint; (void)w; (void)h; (void)x; (void)y;
    /* Bind texture SRV to pixel shader and draw quad */
}

void skia_DrawImageRegion(WD2_RenderContext* ctx, int imageId, SkRect src, SkRect dst, float rotation, SkColor tint) {
    if (!ctx) return;
    if (imageId < 0 || imageId >= ctx->imageCount) return;
    SkImageEntry* img = &ctx->images[imageId];
    if (!img->srv) return;
    (void)src; (void)dst; (void)rotation; (void)tint;
    /* Draw sub-region of image */
}

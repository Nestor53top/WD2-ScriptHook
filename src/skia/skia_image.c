#include "skia.h"
#include <stdlib.h>
#include <string.h>

#define MAX_IMAGES 64

typedef struct {
    int id;
    int width, height;
    ID3D11Texture2D* texture;
    ID3D11ShaderResourceView* srv;
} ImageEntry;

struct WD2_ImageContext {
    ImageEntry images[MAX_IMAGES];
    int imageCount;
    ID3D11Device* device;
    ID3D11DeviceContext* context;
};

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
    int w, h, comp;
    unsigned char* pixels = NULL;
    /* stbi_load_from_memory(data, size, &w, &h, &comp, 4); */
    free(data);
    if (outW) *outW = w;
    if (outH) *outH = h;
    return pixels;
}

static void CreateImageTexture(WD2_ImageContext* ictx, ImageEntry* img, unsigned char* pixels) {
    if (!ictx->device || !pixels) return;
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
    ictx->device->lpVtbl->CreateTexture2D(ictx->device, &desc, &initData, &img->texture);
    ictx->device->lpVtbl->CreateShaderResourceView(ictx->device, (ID3D11Resource*)img->texture, NULL, &img->srv);
}

int skia_LoadImage(WD2_RenderContext* ctx, const char* path) {
    if (!ctx || !path) return -1;
    WD2_ImageContext* ictx = (WD2_ImageContext*)ctx;
    if (ictx->imageCount >= MAX_IMAGES) return -1;

    int w, h;
    unsigned char* pixels = LoadPNGFile(path, &w, &h);
    if (!pixels) return -1;

    ImageEntry* img = &ictx->images[ictx->imageCount];
    memset(img, 0, sizeof(ImageEntry));
    img->id = ictx->imageCount;
    img->width = w;
    img->height = h;
    CreateImageTexture(ictx, img, pixels);
    free(pixels);

    return ictx->imageCount++;
}

void skia_DrawImage(WD2_RenderContext* ctx, int imageId, float x, float y, float w, float h, float rotation, SkColor tint) {
    if (!ctx) return;
    WD2_ImageContext* ictx = (WD2_ImageContext*)ctx;
    if (imageId < 0 || imageId >= ictx->imageCount) return;
    ImageEntry* img = &ictx->images[imageId];
    if (!img->srv) return;
    (void)rotation; (void)tint; (void)w; (void)h; (void)x; (void)y;
    /* Bind texture SRV to pixel shader and draw quad */
}

void skia_DrawImageRegion(WD2_RenderContext* ctx, int imageId, SkRect src, SkRect dst, float rotation, SkColor tint) {
    if (!ctx) return;
    WD2_ImageContext* ictx = (WD2_ImageContext*)ctx;
    if (imageId < 0 || imageId >= ictx->imageCount) return;
    ImageEntry* img = &ictx->images[imageId];
    if (!img->srv) return;
    (void)src; (void)dst; (void)rotation; (void)tint;
    /* Draw sub-region of image */
}

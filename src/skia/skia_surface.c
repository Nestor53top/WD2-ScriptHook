#include "skia.h"
#include <stdlib.h>
#include <string.h>

#define MAX_SURFACES 8

typedef struct {
    int id;
    int width, height;
    ID3D11Texture2D* texture;
    ID3D11RenderTargetView* rtv;
    ID3D11ShaderResourceView* srv;
    int isValid;
} SurfaceEntry;

struct WD2_SurfaceContext {
    SurfaceEntry surfaces[MAX_SURFACES];
    int surfaceCount;
    ID3D11Device* device;
    ID3D11DeviceContext* context;
    int defaultWidth, defaultHeight;
};

static void CreateSurfaceTexture(WD2_SurfaceContext* sctx, SurfaceEntry* surf) {
    if (!sctx->device) return;
    D3D11_TEXTURE2D_DESC desc = {0};
    desc.Width = surf->width;
    desc.Height = surf->height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    sctx->device->lpVtbl->CreateTexture2D(sctx->device, &desc, NULL, &surf->texture);
    sctx->device->lpVtbl->CreateRenderTargetView(sctx->device, (ID3D11Resource*)surf->texture, NULL, &surf->rtv);
    sctx->device->lpVtbl->CreateShaderResourceView(sctx->device, (ID3D11Resource*)surf->texture, NULL, &surf->srv);
    surf->isValid = 1;
}

int skia_CreateSurface(WD2_RenderContext* ctx, int width, int height) {
    if (!ctx) return -1;
    WD2_SurfaceContext* sctx = (WD2_SurfaceContext*)ctx;
    if (sctx->surfaceCount >= MAX_SURFACES) return -1;

    SurfaceEntry* surf = &sctx->surfaces[sctx->surfaceCount];
    memset(surf, 0, sizeof(SurfaceEntry));
    surf->id = sctx->surfaceCount;
    surf->width = width > 0 ? width : sctx->defaultWidth;
    surf->height = height > 0 ? height : sctx->defaultHeight;
    CreateSurfaceTexture(sctx, surf);

    return sctx->surfaceCount++;
}

void skia_DestroySurface(WD2_RenderContext* ctx, int surfaceId) {
    if (!ctx) return;
    WD2_SurfaceContext* sctx = (WD2_SurfaceContext*)ctx;
    if (surfaceId < 0 || surfaceId >= sctx->surfaceCount) return;
    SurfaceEntry* surf = &sctx->surfaces[surfaceId];
    if (surf->texture) { surf->texture->lpVtbl->Release(surf->texture); surf->texture = NULL; }
    if (surf->rtv) { surf->rtv->lpVtbl->Release(surf->rtv); surf->rtv = NULL; }
    if (surf->srv) { surf->srv->lpVtbl->Release(surf->srv); surf->srv = NULL; }
    surf->isValid = 0;
}

void skia_SetRenderTarget(WD2_RenderContext* ctx, int surfaceId) {
    if (!ctx) return;
    WD2_SurfaceContext* sctx = (WD2_SurfaceContext*)ctx;
    if (surfaceId < 0 || surfaceId >= sctx->surfaceCount) return;
    SurfaceEntry* surf = &sctx->surfaces[surfaceId];
    if (!surf->isValid) return;
    sctx->context->lpVtbl->OMSetRenderTargets(sctx->context, 1, &surf->rtv, NULL);
    D3D11_VIEWPORT vp = {0, 0, (float)surf->width, (float)surf->height, 0.0f, 1.0f};
    sctx->context->lpVtbl->RSSetViewports(sctx->context, 1, &vp);
}

void skia_ResetRenderTarget(WD2_RenderContext* ctx) {
    if (!ctx) return;
    WD2_RenderContext* rctx = ctx;
    rctx->context->lpVtbl->OMSetRenderTargets(rctx->context, 1, &rctx->renderTargetView, NULL);
}

int skia_GetSurfaceTexture(WD2_RenderContext* ctx, int surfaceId, ID3D11ShaderResourceView** outSrv) {
    if (!ctx || !outSrv) return 0;
    WD2_SurfaceContext* sctx = (WD2_SurfaceContext*)ctx;
    if (surfaceId < 0 || surfaceId >= sctx->surfaceCount) return 0;
    SurfaceEntry* surf = &sctx->surfaces[surfaceId];
    if (!surf->isValid || !surf->srv) return 0;
    *outSrv = surf->srv;
    return 1;
}

void skia_ClearSurface(WD2_RenderContext* ctx, int surfaceId, SkColor color) {
    if (!ctx) return;
    WD2_SurfaceContext* sctx = (WD2_SurfaceContext*)ctx;
    if (surfaceId < 0 || surfaceId >= sctx->surfaceCount) return;
    SurfaceEntry* surf = &sctx->surfaces[surfaceId];
    if (!surf->isValid) return;
    float clearColor[4] = {color.r, color.g, color.b, color.a};
    sctx->context->lpVtbl->ClearRenderTargetView(sctx->context, surf->rtv, clearColor);
}

int skia_GetSurfaceWidth(WD2_RenderContext* ctx, int surfaceId) {
    if (!ctx) return 0;
    WD2_SurfaceContext* sctx = (WD2_SurfaceContext*)ctx;
    if (surfaceId < 0 || surfaceId >= sctx->surfaceCount) return 0;
    return sctx->surfaces[surfaceId].width;
}

int skia_GetSurfaceHeight(WD2_RenderContext* ctx, int surfaceId) {
    if (!ctx) return 0;
    WD2_SurfaceContext* sctx = (WD2_SurfaceContext*)ctx;
    if (surfaceId < 0 || surfaceId >= sctx->surfaceCount) return 0;
    return sctx->surfaces[surfaceId].height;
}

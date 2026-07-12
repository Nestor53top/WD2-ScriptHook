#include "skia.h"

static void CreateSurfaceTexture(WD2_RenderContext* rctx, SkSurfaceEntry* surf) {
    if (!rctx->device) return;
    D3D11_TEXTURE2D_DESC desc = {0};
    desc.Width = surf->width;
    desc.Height = surf->height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    rctx->device->lpVtbl->CreateTexture2D(rctx->device, &desc, NULL, &surf->texture);
    rctx->device->lpVtbl->CreateRenderTargetView(rctx->device, (ID3D11Resource*)surf->texture, NULL, &surf->rtv);
    rctx->device->lpVtbl->CreateShaderResourceView(rctx->device, (ID3D11Resource*)surf->texture, NULL, &surf->srv);
    surf->isValid = 1;
}

int skia_CreateSurface(WD2_RenderContext* ctx, int width, int height) {
    if (!ctx) return -1;
    if (ctx->surfaceCount >= MAX_SURFACES) return -1;

    SkSurfaceEntry* surf = &ctx->surfaces[ctx->surfaceCount];
    memset(surf, 0, sizeof(SkSurfaceEntry));
    surf->id = ctx->surfaceCount;
    surf->width = width > 0 ? width : ctx->defaultWidth;
    surf->height = height > 0 ? height : ctx->defaultHeight;
    CreateSurfaceTexture(ctx, surf);

    return ctx->surfaceCount++;
}

void skia_DestroySurface(WD2_RenderContext* ctx, int surfaceId) {
    if (!ctx) return;
    if (surfaceId < 0 || surfaceId >= ctx->surfaceCount) return;
    SkSurfaceEntry* surf = &ctx->surfaces[surfaceId];
    if (surf->texture) { surf->texture->lpVtbl->Release(surf->texture); surf->texture = NULL; }
    if (surf->rtv) { surf->rtv->lpVtbl->Release(surf->rtv); surf->rtv = NULL; }
    if (surf->srv) { surf->srv->lpVtbl->Release(surf->srv); surf->srv = NULL; }
    surf->isValid = 0;
}

void skia_SetRenderTarget(WD2_RenderContext* ctx, int surfaceId) {
    if (!ctx) return;
    if (surfaceId < 0 || surfaceId >= ctx->surfaceCount) return;
    SkSurfaceEntry* surf = &ctx->surfaces[surfaceId];
    if (!surf->isValid) return;
    ctx->context->lpVtbl->OMSetRenderTargets(ctx->context, 1, &surf->rtv, NULL);
    D3D11_VIEWPORT vp = {0, 0, (float)surf->width, (float)surf->height, 0.0f, 1.0f};
    ctx->context->lpVtbl->RSSetViewports(ctx->context, 1, &vp);
}

void skia_ResetRenderTarget(WD2_RenderContext* ctx) {
    if (!ctx) return;
    ctx->context->lpVtbl->OMSetRenderTargets(ctx->context, 1, &ctx->renderTargetView, NULL);
}

int skia_GetSurfaceTexture(WD2_RenderContext* ctx, int surfaceId, ID3D11ShaderResourceView** outSrv) {
    if (!ctx || !outSrv) return 0;
    if (surfaceId < 0 || surfaceId >= ctx->surfaceCount) return 0;
    SkSurfaceEntry* surf = &ctx->surfaces[surfaceId];
    if (!surf->isValid || !surf->srv) return 0;
    *outSrv = surf->srv;
    return 1;
}

void skia_ClearSurface(WD2_RenderContext* ctx, int surfaceId, SkColor color) {
    if (!ctx) return;
    if (surfaceId < 0 || surfaceId >= ctx->surfaceCount) return;
    SkSurfaceEntry* surf = &ctx->surfaces[surfaceId];
    if (!surf->isValid) return;
    float clearColor[4] = {color.r, color.g, color.b, color.a};
    ctx->context->lpVtbl->ClearRenderTargetView(ctx->context, surf->rtv, clearColor);
}

int skia_GetSurfaceWidth(WD2_RenderContext* ctx, int surfaceId) {
    if (!ctx) return 0;
    if (surfaceId < 0 || surfaceId >= ctx->surfaceCount) return 0;
    return ctx->surfaces[surfaceId].width;
}

int skia_GetSurfaceHeight(WD2_RenderContext* ctx, int surfaceId) {
    if (!ctx) return 0;
    if (surfaceId < 0 || surfaceId >= ctx->surfaceCount) return 0;
    return ctx->surfaces[surfaceId].height;
}

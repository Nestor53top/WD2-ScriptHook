#include "skia.h"
#include <d3dcompiler.h>
#include <string.h>
#include <stdlib.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

typedef struct {
    float x, y, z;
    float r, g, b, a;
} Vertex;

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
} WD2_RenderContext;

static const char* vertexShaderSrc =
    "cbuffer ConstBuffer : register(b0) {\n"
    "    float4x4 Proj;\n"
    "};\n"
    "struct VS_INPUT {\n"
    "    float3 Pos : POSITION;\n"
    "    float4 Color : COLOR;\n"
    "};\n"
    "struct VS_OUTPUT {\n"
    "    float4 Pos : SV_POSITION;\n"
    "    float4 Color : COLOR;\n"
    "};\n"
    "VS_OUTPUT main(VS_INPUT input) {\n"
    "    VS_OUTPUT output;\n"
    "    output.Pos = mul(float4(input.Pos, 1.0f), Proj);\n"
    "    output.Color = input.Color;\n"
    "    return output;\n"
    "}\n";

static const char* pixelShaderSrc =
    "struct PS_INPUT {\n"
    "    float4 Pos : SV_POSITION;\n"
    "    float4 Color : COLOR;\n"
    "};\n"
    "float4 main(PS_INPUT input) : SV_TARGET {\n"
    "    return input.Color;\n"
    "}\n";

static void CreateOrthoMatrix(float w, float h, float* out) {
    memset(out, 0, 16 * sizeof(float));
    out[0] = 2.0f / w;
    out[5] = -2.0f / h;
    out[10] = 1.0f;
    out[12] = -1.0f;
    out[13] = 1.0f;
    out[15] = 1.0f;
}

static int CompileShader(ID3D11Device* dev, const char* src, const char* target, ID3D11ShaderReflection** refOut, ID3DBlob** blobOut) {
    ID3DBlob* blob = NULL, *errBlob = NULL;
    HRESULT hr = D3DCompile(src, strlen(src), NULL, NULL, NULL, "main", target, D3DCOMPILE_ENABLE_STRICTNESS, 0, &blob, &errBlob);
    if (errBlob) errBlob->lpVtbl->Release(errBlob);
    if (FAILED(hr)) return 0;
    if (blobOut) *blobOut = blob;
    return 1;
}

static void SetupRenderTargets(WD2_RenderContext* ctx) {
    ID3D11Texture2D* backBuffer = NULL;
    ctx->swapChain->lpVtbl->GetBuffer(ctx->swapChain, 0, &IID_ID3D11Texture2D, (void**)&backBuffer);
    if (backBuffer) {
        ctx->device->lpVtbl->CreateRenderTargetView(ctx->device, (ID3D11Resource*)backBuffer, NULL, &ctx->renderTargetView);
        backBuffer->lpVtbl->Release(backBuffer);
    }
}

static void CreateDefaultStates(WD2_RenderContext* ctx) {
    D3D11_BLEND_DESC blendDesc = {0};
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    ctx->device->lpVtbl->CreateBlendState(ctx->device, &blendDesc, &ctx->blendState);

    D3D11_RASTERIZER_DESC rastDesc = {0};
    rastDesc.FillMode = D3D11_FILL_SOLID;
    rastDesc.CullMode = D3D11_CULL_NONE;
    rastDesc.DepthClipEnable = FALSE;
    ctx->device->lpVtbl->CreateRasterizerState(ctx->device, &rastDesc, &ctx->rasterizerState);

    D3D11_DEPTH_STENCIL_DESC dsDesc = {0};
    dsDesc.DepthEnable = FALSE;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    ctx->device->lpVtbl->CreateDepthStencilState(ctx->device, &dsDesc, &ctx->depthStencilState);
}

static void CreateShaders(WD2_RenderContext* ctx) {
    ID3DBlob* vsBlob = NULL, *psBlob = NULL;
    if (CompileShader(ctx->device, vertexShaderSrc, "vs_5_0", NULL, &vsBlob)) {
        ctx->device->lpVtbl->CreateVertexShader(ctx->device, vsBlob->lpVtbl->GetBufferPointer(vsBlob), vsBlob->lpVtbl->GetBufferSize(vsBlob), NULL, &ctx->vertexShader);
        D3D11_INPUT_ELEMENT_DESC layout[] = {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
        };
        ctx->device->lpVtbl->CreateInputLayout(ctx->device, layout, 2, vsBlob->lpVtbl->GetBufferPointer(vsBlob), vsBlob->lpVtbl->GetBufferSize(vsBlob), &ctx->inputLayout);
        vsBlob->lpVtbl->Release(vsBlob);
    }
    if (CompileShader(ctx->device, pixelShaderSrc, "ps_5_0", NULL, &psBlob)) {
        ctx->device->lpVtbl->CreatePixelShader(ctx->device, psBlob->lpVtbl->GetBufferPointer(psBlob), psBlob->lpVtbl->GetBufferSize(psBlob), NULL, &ctx->pixelShader);
        psBlob->lpVtbl->Release(psBlob);
    }
}

static void CreateBuffers(WD2_RenderContext* ctx) {
    ctx->vertexBufferCapacity = 1024;
    D3D11_BUFFER_DESC vbd = {0};
    vbd.Usage = D3D11_USAGE_DYNAMIC;
    vbd.ByteWidth = sizeof(Vertex) * ctx->vertexBufferCapacity;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    ctx->device->lpVtbl->CreateBuffer(ctx->device, &vbd, NULL, &ctx->vertexBuffer);

    D3D11_BUFFER_DESC ibd = {0};
    ibd.Usage = D3D11_USAGE_DYNAMIC;
    ibd.ByteWidth = sizeof(unsigned short) * ctx->vertexBufferCapacity * 3;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    ctx->device->lpVtbl->CreateBuffer(ctx->device, &ibd, NULL, &ctx->indexBuffer);

    D3D11_BUFFER_DESC cbd = {0};
    cbd.Usage = D3D11_USAGE_DEFAULT;
    cbd.ByteWidth = 64;
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    ctx->device->lpVtbl->CreateBuffer(ctx->device, &cbd, NULL, &ctx->constantBuffer);
}

WD2_RenderContext* skia_CreateRenderContext(ID3D11Device* dev, ID3D11DeviceContext* ctx, IDXGISwapChain* sc) {
    WD2_RenderContext* rctx = (WD2_RenderContext*)calloc(1, sizeof(WD2_RenderContext));
    if (!rctx) return NULL;
    rctx->device = dev;
    rctx->context = ctx;
    rctx->swapChain = sc;
    dev->lpVtbl->AddRef(dev);
    ctx->lpVtbl->AddRef(ctx);
    sc->lpVtbl->AddRef(sc);

    SetupRenderTargets(rctx);
    CreateDefaultStates(rctx);
    CreateShaders(rctx);
    CreateBuffers(rctx);

    IDXGISwapChain* swapChain = sc;
    IDXGIOutput* output = NULL;
    swapChain->lpVtbl->GetContainingOutput(swapChain, &output);
    if (output) {
        DXGI_OUTPUT_DESC desc;
        output->lpVtbl->GetDesc(output, &desc);
        rctx->width = desc.DesktopCoordinates.right - desc.DesktopCoordinates.left;
        rctx->height = desc.DesktopCoordinates.bottom - desc.DesktopCoordinates.top;
        output->lpVtbl->Release(output);
    } else {
        rctx->width = 1920;
        rctx->height = 1080;
    }
    return rctx;
}

void skia_DestroyRenderContext(WD2_RenderContext* ctx) {
    if (!ctx) return;
    if (ctx->vertexShader) ctx->vertexShader->lpVtbl->Release(ctx->vertexShader);
    if (ctx->pixelShader) ctx->pixelShader->lpVtbl->Release(ctx->pixelShader);
    if (ctx->inputLayout) ctx->inputLayout->lpVtbl->Release(ctx->inputLayout);
    if (ctx->vertexBuffer) ctx->vertexBuffer->lpVtbl->Release(ctx->vertexBuffer);
    if (ctx->indexBuffer) ctx->indexBuffer->lpVtbl->Release(ctx->indexBuffer);
    if (ctx->constantBuffer) ctx->constantBuffer->lpVtbl->Release(ctx->constantBuffer);
    if (ctx->blendState) ctx->blendState->lpVtbl->Release(ctx->blendState);
    if (ctx->rasterizerState) ctx->rasterizerState->lpVtbl->Release(ctx->rasterizerState);
    if (ctx->depthStencilState) ctx->depthStencilState->lpVtbl->Release(ctx->depthStencilState);
    if (ctx->renderTargetView) ctx->renderTargetView->lpVtbl->Release(ctx->renderTargetView);
    if (ctx->samplerState) ctx->samplerState->lpVtbl->Release(ctx->samplerState);
    if (ctx->fontTexture) ctx->fontTexture->lpVtbl->Release(ctx->fontTexture);
    if (ctx->fontTextureSRV) ctx->fontTextureSRV->lpVtbl->Release(ctx->fontTextureSRV);
    ctx->device->lpVtbl->Release(ctx->device);
    ctx->context->lpVtbl->Release(ctx->context);
    ctx->swapChain->lpVtbl->Release(ctx->swapChain);
    free(ctx);
}

void skia_OnResize(WD2_RenderContext* ctx, int w, int h) {
    if (!ctx) return;
    if (ctx->renderTargetView) {
        ctx->renderTargetView->lpVtbl->Release(ctx->renderTargetView);
        ctx->renderTargetView = NULL;
    }
    ctx->width = w;
    ctx->height = h;
    ctx->swapChain->lpVtbl->ResizeBuffers(ctx->swapChain, 0, w, h, DXGI_FORMAT_UNKNOWN, 0);
    SetupRenderTargets(ctx);
}

void skia_BeginFrame(WD2_RenderContext* ctx) {
    if (!ctx) return;
    ctx->vertexCount = 0;
    ctx->indexCount = 0;
    float clearColor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    ctx->context->lpVtbl->OMSetRenderTargets(ctx->context, 1, &ctx->renderTargetView, NULL);
    ctx->context->lpVtbl->ClearRenderTargetView(ctx->context, ctx->renderTargetView, clearColor);
    ctx->context->lpVtbl->RSSetViewports(ctx->context, 1, &(D3D11_VIEWPORT){0, 0, (float)ctx->width, (float)ctx->height, 0.0f, 1.0f});
    ctx->context->lpVtbl->IASetInputLayout(ctx->context, ctx->inputLayout);
    ctx->context->lpVtbl->IASetPrimitiveTopology(ctx->context, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx->context->lpVtbl->VSSetShader(ctx->context, ctx->vertexShader, NULL, 0);
    ctx->context->lpVtbl->PSSetShader(ctx->context, ctx->pixelShader, NULL, 0);
    ctx->context->lpVtbl->OMSetBlendState(ctx->context, ctx->blendState, NULL, 0xFFFFFFFF);
    ctx->context->lpVtbl->RSSetState(ctx->context, ctx->rasterizerState);
    ctx->context->lpVtbl->OMSetDepthStencilState(ctx->context, ctx->depthStencilState, 0);
}

static void FlushBatch(WD2_RenderContext* ctx) {
    if (ctx->vertexCount == 0 || ctx->indexCount == 0) return;
    D3D11_MAPPED_SUBRESOURCE mapped;
    if (SUCCEEDED(ctx->context->lpVtbl->Map(ctx->context, (ID3D11Resource*)ctx->vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
        memcpy(mapped.pData, /* vertices */ NULL, ctx->vertexCount * sizeof(Vertex));
        ctx->context->lpVtbl->Unmap(ctx->context, (ID3D11Resource*)ctx->vertexBuffer, 0);
    }
    if (SUCCEEDED(ctx->context->lpVtbl->Map(ctx->context, (ID3D11Resource*)ctx->indexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
        memcpy(mapped.pData, /* indices */ NULL, ctx->indexCount * sizeof(unsigned short));
        ctx->context->lpVtbl->Unmap(ctx->context, (ID3D11Resource*)ctx->indexBuffer, 0);
    }
    float proj[16];
    CreateOrthoMatrix((float)ctx->width, (float)ctx->height, proj);
    ctx->context->lpVtbl->UpdateSubresource(ctx->context, (ID3D11Resource*)ctx->constantBuffer, 0, NULL, proj, 0, 0);
    UINT stride = sizeof(Vertex), offset = 0;
    ctx->context->lpVtbl->IASetVertexBuffers(ctx->context, 0, 1, &ctx->vertexBuffer, &stride, &offset);
    ctx->context->lpVtbl->IASetIndexBuffer(ctx->context, ctx->indexBuffer, DXGI_FORMAT_R16_UINT, 0);
    ctx->context->lpVtbl->VSSetConstantBuffers(ctx->context, 0, 1, &ctx->constantBuffer);
    ctx->context->lpVtbl->DrawIndexed(ctx->context, ctx->indexCount, 0, 0);
    ctx->vertexCount = 0;
    ctx->indexCount = 0;
}

void skia_EndFrame(WD2_RenderContext* ctx) {
    if (!ctx) return;
    FlushBatch(ctx);
}

static void AddQuadVertices(WD2_RenderContext* ctx, float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, SkColor color) {
    Vertex verts[4] = {
        {x1, y1, 0.0f, color.r, color.g, color.b, color.a},
        {x2, y2, 0.0f, color.r, color.g, color.b, color.a},
        {x3, y3, 0.0f, color.r, color.g, color.b, color.a},
        {x4, y4, 0.0f, color.r, color.g, color.b, color.a}
    };
    /* Add to vertex buffer */
}

void skia_DrawRect(WD2_RenderContext* ctx, SkRect rect, SkColor color) {
    if (!ctx) return;
    /* Draw outline only - 4 lines */
}

void skia_DrawRectFilled(WD2_RenderContext* ctx, SkRect rect, SkColor color) {
    if (!ctx) return;
    AddQuadVertices(ctx, rect.x, rect.y, rect.x + rect.w, rect.y, rect.x + rect.w, rect.y + rect.h, rect.x, rect.y + rect.h, color);
}

void skia_DrawLine(WD2_RenderContext* ctx, float x1, float y1, float x2, float y2, SkColor color, float thickness) {
    if (!ctx) return;
    float dx = x2 - x1, dy = y2 - y1;
    float len = sqrtf(dx * dx + dy * dy);
    if (len < 0.0001f) return;
    float nx = -dy / len * thickness * 0.5f;
    float ny = dx / len * thickness * 0.5f;
    AddQuadVertices(ctx, x1 + nx, y1 + ny, x2 + nx, y2 + ny, x2 - nx, y2 - ny, x1 - nx, y1 - ny, color);
}

void skia_DrawCircle(WD2_RenderContext* ctx, float cx, float cy, float radius, SkColor color, float thickness) {
    if (!ctx) return;
    /* Approximate circle with line segments */
}

void skia_DrawCircleFilled(WD2_RenderContext* ctx, float cx, float cy, float radius, SkColor color) {
    if (!ctx) return;
    /* Approximate circle with triangle fan */
}

void skia_DrawTriangle(WD2_RenderContext* ctx, float x1, float y1, float x2, float y2, float x3, float y3, SkColor color) {
    if (!ctx) return;
    Vertex verts[3] = {
        {x1, y1, 0.0f, color.r, color.g, color.b, color.a},
        {x2, y2, 0.0f, color.r, color.g, color.b, color.a},
        {x3, y3, 0.0f, color.r, color.g, color.b, color.a}
    };
    /* Add to vertex buffer */
}

void skia_SetClipRect(WD2_RenderContext* ctx, SkRect rect) {
    if (!ctx) return;
    D3D11_RECT clip = {(LONG)rect.x, (LONG)rect.y, (LONG)(rect.x + rect.w), (LONG)(rect.y + rect.h)};
    ctx->context->lpVtbl->RSSetScissorRects(ctx->context, 1, &clip);
}

void skia_ClearClip(WD2_RenderContext* ctx) {
    if (!ctx) return;
    D3D11_RECT full = {0, 0, ctx->width, ctx->height};
    ctx->context->lpVtbl->RSSetScissorRects(ctx->context, 1, &full);
}

void skia_DrawText(WD2_RenderContext* ctx, float x, float y, const char* text, SkColor color, float size) {
    if (!ctx || !text) return;
    /* Text rendering via font atlas */
}

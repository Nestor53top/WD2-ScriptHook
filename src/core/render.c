#define CINTERFACE
#include "render.h"
#include "hook_manager.h"
#include <d3d11.h>
#include <dxgi.h>

#pragma comment(lib, "d3d11.lib")

typedef struct _RENDER_STATE {
    ID3D11Device           *pDevice;
    ID3D11DeviceContext    *pContext;
    IDXGISwapChain         *pSwapChain;
    ID3D11RenderTargetView *pRenderTarget;
    ID3D11BlendState       *pBlendState;
    ID3D11DepthStencilState *pDepthStencil;
    ID3D11RasterizerState  *pRasterizer;
    ID3D11VertexShader     *pVertexShader;
    ID3D11PixelShader      *pPixelShader;
    ID3D11InputLayout      *pInputLayout;
    ID3D11Buffer           *pVertexBuffer;
    ID3D11Buffer           *pConstantBuffer;
    UINT                    uWidth;
    UINT                    uHeight;
    BOOL                    bInitialized;
    BOOL                    bFrameInProgress;
} RENDER_STATE;

static RENDER_STATE g_render = {0};

typedef HRESULT (WINAPI *Present_t)(IDXGISwapChain *pSwapChain, UINT SyncInterval, UINT Flags);
typedef HRESULT (WINAPI *ResizeBuffers_t)(IDXGISwapChain *pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);

static Present_t      g_origPresent = NULL;
static ResizeBuffers_t g_origResizeBuffers = NULL;

static HRESULT WINAPI HookedPresent(IDXGISwapChain *pSwapChain, UINT SyncInterval, UINT Flags) {
    if (!g_render.bInitialized) {
        IDXGISwapChain_GetDevice(pSwapChain, &IID_ID3D11Device, (void **)&g_render.pDevice);
        if (g_render.pDevice) {
            ID3D11Device_GetImmediateContext(g_render.pDevice, &g_render.pContext);
            g_render.pSwapChain = pSwapChain;
            wd2_render_init(pSwapChain);
        }
    }

    if (g_render.bInitialized && !g_render.bFrameInProgress) {
        g_render.bFrameInProgress = TRUE;
        wd2_render_begin_frame();
        wd2_lua_on_render();
        wd2_render_end_frame();
        g_render.bFrameInProgress = FALSE;
    }

    return g_origPresent(pSwapChain, SyncInterval, Flags);
}

static HRESULT WINAPI HookedResizeBuffers(IDXGISwapChain *pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags) {
    wd2_render_shutdown();
    HRESULT hr = g_origResizeBuffers(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
    if (SUCCEEDED(hr)) {
        wd2_render_on_resize(Width, Height);
    }
    return hr;
}

BOOL wd2_render_init(IDXGISwapChain *pSwapChain) {
    if (g_render.bInitialized) return TRUE;

    wd2_log_info("Initializing D3D11 renderer");

    g_render.pSwapChain = pSwapChain;
    if (!g_render.pDevice) {
        IDXGISwapChain_GetDevice(pSwapChain, &IID_ID3D11Device, (void **)&g_render.pDevice);
    }
    if (!g_render.pContext && g_render.pDevice) {
        ID3D11Device_GetImmediateContext(g_render.pDevice, &g_render.pContext);
    }
    if (!g_render.pDevice || !g_render.pContext) {
        wd2_log_error("Failed to get D3D11 device/context");
        return FALSE;
    }

    IDXGISwapChain_GetBuffer(pSwapChain, 0, &IID_ID3D11Texture2D, (void **)&g_render.pRenderTarget);
    if (g_render.pRenderTarget) {
        ID3D11Device_CreateRenderTargetView(g_render.pDevice,
            (ID3D11Resource *)g_render.pRenderTarget, NULL, &g_render.pRenderTarget);
    }

    DXGI_SWAP_CHAIN_DESC desc;
    IDXGISwapChain_GetDesc(pSwapChain, &desc);
    g_render.uWidth = desc.BufferDesc.Width;
    g_render.uHeight = desc.BufferDesc.Height;

    D3D11_BLEND_DESC blend_desc = {0};
    blend_desc.RenderTarget[0].BlendEnable = TRUE;
    blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    ID3D11Device_CreateBlendState(g_render.pDevice, &blend_desc, &g_render.pBlendState);

    D3D11_DEPTH_STENCIL_DESC ds_desc = {0};
    ds_desc.DepthEnable = FALSE;
    ds_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    ID3D11Device_CreateDepthStencilState(g_render.pDevice, &ds_desc, &g_render.pDepthStencil);

    D3D11_RASTERIZER_DESC rs_desc = {0};
    rs_desc.FillMode = D3D11_FILL_SOLID;
    rs_desc.CullMode = D3D11_CULL_NONE;
    rs_desc.DepthClipEnable = FALSE;
    ID3D11Device_CreateRasterizerState(g_render.pDevice, &rs_desc, &g_render.pRasterizer);

    g_render.bInitialized = TRUE;
    wd2_log_info("D3D11 renderer initialized (%ux%u)", g_render.uWidth, g_render.uHeight);
    return TRUE;
}

void wd2_render_shutdown(void) {
    if (!g_render.bInitialized) return;

    wd2_log_info("Shutting down D3D11 renderer");

    SAFE_RELEASE(g_render.pRasterizer);
    SAFE_RELEASE(g_render.pDepthStencil);
    SAFE_RELEASE(g_render.pBlendState);
    SAFE_RELEASE(g_render.pRenderTarget);
    SAFE_RELEASE(g_render.pVertexBuffer);
    SAFE_RELEASE(g_render.pConstantBuffer);
    SAFE_RELEASE(g_render.pInputLayout);
    SAFE_RELEASE(g_render.pPixelShader);
    SAFE_RELEASE(g_render.pVertexShader);
    SAFE_RELEASE(g_render.pContext);
    SAFE_RELEASE(g_render.pDevice);

    g_render.bInitialized = FALSE;
}

void wd2_render_begin_frame(void) {
    if (!g_render.bInitialized) return;
    if (!g_render.pContext) return;

    float clear[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    ID3D11DeviceContext_ClearRenderTargetView(g_render.pContext, g_render.pRenderTarget, clear);
    ID3D11DeviceContext_OMSetRenderTargets(g_render.pContext, 1, &g_render.pRenderTarget, NULL);

    D3D11_VIEWPORT vp = {0};
    vp.Width = (float)g_render.uWidth;
    vp.Height = (float)g_render.uHeight;
    vp.MaxDepth = 1.0f;
    ID3D11DeviceContext_RSSetViewports(g_render.pContext, 1, &vp);
}

void wd2_render_end_frame(void) {
    if (!g_render.bInitialized) return;
}

void wd2_render_draw_text(float x, float y, const char *text, float r, float g, float b, float a, float size) {
    if (!g_render.bInitialized || !text) return;
    (void)x; (void)y; (void)r; (void)g; (void)b; (void)a; (void)size;
}

void wd2_render_draw_rect(float x, float y, float w, float h, float r, float g, float b, float a) {
    if (!g_render.bInitialized) return;

    typedef struct { float x, y, z, w; } Vertex;

    Vertex vertices[6] = {
        {x,     y,     0.0f, 1.0f},
        {x + w, y,     0.0f, 1.0f},
        {x,     y + h, 0.0f, 1.0f},
        {x + w, y,     0.0f, 1.0f},
        {x + w, y + h, 0.0f, 1.0f},
        {x,     y + h, 0.0f, 1.0f},
    };

    D3D11_BUFFER_DESC bd = {0};
    bd.ByteWidth = sizeof(vertices);
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA init_data = {0};
    init_data.pSysMem = vertices;

    ID3D11Buffer *vb = NULL;
    if (FAILED(ID3D11Device_CreateBuffer(g_render.pDevice, &bd, &init_data, &vb))) return;

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    ID3D11DeviceContext_IASetVertexBuffers(g_render.pContext, 0, 1, &vb, &stride, &offset);
    ID3D11DeviceContext_IASetPrimitiveTopology(g_render.pContext, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    if (g_render.pBlendState) {
        float blend_factor[4] = {r, g, b, a};
        ID3D11DeviceContext_OMSetBlendState(g_render.pContext, g_render.pBlendState, blend_factor, 0xFFFFFFFF);
    }
    if (g_render.pDepthStencil) {
        ID3D11DeviceContext_OMSetDepthStencilState(g_render.pContext, g_render.pDepthStencil, 0);
    }
    if (g_render.pRasterizer) {
        ID3D11DeviceContext_RSSetState(g_render.pContext, g_render.pRasterizer);
    }

    ID3D11DeviceContext_Draw(g_render.pContext, 6, 0);
    SAFE_RELEASE(vb);
}

void wd2_render_draw_line(float x1, float y1, float x2, float y2, float r, float g, float b, float a, float thickness) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    float len = sqrtf(dx * dx + dy * dy);
    if (len < 0.001f) return;

    float nx = -dy / len * thickness * 0.5f;
    float ny = dx / len * thickness * 0.5f;

    wd2_render_draw_rect(
        x1 + nx, y1 + ny,
        x2 - x1, y2 - y1,
        r, g, b, a);
}

void wd2_render_draw_circle(float cx, float cy, float radius, float r, float g, float b, float a) {
    if (!g_render.bInitialized) return;

    int segments = 32;
    typedef struct { float x, y, z, w; } Vertex;

    Vertex vertices[32 * 3];
    for (int i = 0; i < segments; i++) {
        float a1 = (float)i / segments * 6.2831853f;
        float a2 = (float)(i + 1) / segments * 6.2831853f;
        vertices[i * 3 + 0] = (Vertex){cx, cy, 0.0f, 1.0f};
        vertices[i * 3 + 1] = (Vertex){cx + cosf(a1) * radius, cy + sinf(a1) * radius, 0.0f, 1.0f};
        vertices[i * 3 + 2] = (Vertex){cx + cosf(a2) * radius, cy + sinf(a2) * radius, 0.0f, 1.0f};
    }

    D3D11_BUFFER_DESC bd = {0};
    bd.ByteWidth = sizeof(vertices);
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA init_data = {0};
    init_data.pSysMem = vertices;

    ID3D11Buffer *vb = NULL;
    if (FAILED(ID3D11Device_CreateBuffer(g_render.pDevice, &bd, &init_data, &vb))) return;

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    ID3D11DeviceContext_IASetVertexBuffers(g_render.pContext, 0, 1, &vb, &stride, &offset);
    ID3D11DeviceContext_IASetPrimitiveTopology(g_render.pContext, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    if (g_render.pBlendState) {
        float blend_factor[4] = {r, g, b, a};
        ID3D11DeviceContext_OMSetBlendState(g_render.pContext, g_render.pBlendState, blend_factor, 0xFFFFFFFF);
    }
    if (g_render.pDepthStencil) {
        ID3D11DeviceContext_OMSetDepthStencilState(g_render.pContext, g_render.pDepthStencil, 0);
    }
    if (g_render.pRasterizer) {
        ID3D11DeviceContext_RSSetState(g_render.pContext, g_render.pRasterizer);
    }

    ID3D11DeviceContext_Draw(g_render.pContext, segments * 3, 0);
    SAFE_RELEASE(vb);
}

void wd2_render_on_resize(UINT width, UINT height) {
    g_render.uWidth = width;
    g_render.uHeight = height;
    wd2_log_info("Renderer resized: %ux%u", width, height);
}

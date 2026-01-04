module;
#include <d3d11_4.h>
#include <dcomp.h>
#include <wrl.h>
#include <dwrite_3.h>
#define PRE_MUL_COLOR(R,G,B,A) ((R)*(A)), ((G)*(A)), ((B)*(A)), (A)
module render;

using namespace Editor::Render;

static void GetSystemUIFontInfo(std::wstring& font, float& size)
{
	NONCLIENTMETRICSW ncm = { sizeof(ncm) };
	if (SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0))
	{
		font = ncm.lfMessageFont.lfFaceName;
		if (ncm.lfMessageFont.lfHeight < 0) size = -ncm.lfMessageFont.lfHeight * 72.0f / GetDpiForSystem();
		else size = 9.0f;
	}
	else
	{
		font = L"Segoe UI";
		size = 9.0f;
	}
}

static HRESULT CreateSystemDefaultTextFormat(IDWriteFactory* factory, IDWriteTextFormat** format)
{
	std::wstring font;
	float size;
	GetSystemUIFontInfo(font, size);
	return factory->CreateTextFormat(
		font.c_str(),
		nullptr,
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		size,
		L"",
		format
	);
}

Renderer::Renderer() : background{ PRE_MUL_COLOR(0.6, 0.6, 0.6, 0.8) }, width(1280), height(720)
{
	ComPtr<IDXGIDevice> dxgi;
	D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_1 };
	D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT, levels, 1, D3D11_SDK_VERSION, &d3d, nullptr, &d3d_ctx);
	d3d.As(&dxgi);
	D2D1CreateDevice(dxgi.Get(), {}, &d2d);
	d2d->CreateDeviceContext({}, &d2d_ctx);
	DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), &dwrite);
	CreateSystemDefaultTextFormat(dwrite.Get(), &default_format);
	DCompositionCreateDevice3(nullptr, IID_PPV_ARGS(&dcomp));
	ComPtr<IDXGIFactory4> factory;
	{
		ComPtr<IDXGIAdapter> adapter;
		dxgi->GetAdapter(&adapter);
		adapter->GetParent(IID_PPV_ARGS(&factory));
	}
	{
		ComPtr<IDXGISwapChain1> temp;
		DXGI_SWAP_CHAIN_DESC1 desc;
		desc.BufferCount = 2;
		desc.Width = 128;
		desc.Height = 720;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		desc.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;
		desc.Scaling = DXGI_SCALING_STRETCH;
		desc.Stereo = false;
		factory->CreateSwapChainForComposition(d3d.Get(), &desc, nullptr, &temp);
		temp.As(&swap_chain);
	}
	swap_chain->SetMaximumFrameLatency(1);
	swap_event = swap_chain->GetFrameLatencyWaitableObject();
	D3D11_BLEND_DESC bd{};
	bd.RenderTarget[0].BlendEnable = true;
	bd.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	bd.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	d3d->CreateBlendState(&bd, &blend);
}

void Renderer::BeginRender()
{
	d3d_ctx->OMSetRenderTargets(1, rtv.GetAddressOf(), dsv.Get());
	d3d_ctx->OMSetBlendState(blend.Get(), blend_factor, ~0);
	d3d_ctx->ClearRenderTargetView(rtv.Get(), background);
	d3d_ctx->ClearDepthStencilView(dsv.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
	d2d_ctx->BeginDraw();
}

void Renderer::EndRender() const
{
	d2d_ctx->EndDraw();
	WaitForSingleObject(swap_event, INFINITE);
	swap_chain->Present(1, 0);
}

void Renderer::Bind(HWND window)
{
	ComPtr<IDCompositionVisual2> visual;
	dcomp->CreateTargetForHwnd(window, false, &target);
	dcomp->CreateVisual(&visual);
	target->SetRoot(visual.Get());
	visual->SetContent(swap_chain.Get());
	dcomp->Commit();
}

void Renderer::Resize(int width, int height)
{
	ComPtr<ID3D11Texture2D> buffer;
	ComPtr<IDXGISurface> dxgi;
	rtv = nullptr;
	surface = nullptr;
	d2d_ctx->SetTarget(nullptr);
	swap_chain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT);
	swap_chain->GetBuffer(0, IID_PPV_ARGS(&buffer));
	d3d->CreateRenderTargetView(buffer.Get(), nullptr, &rtv);
	D3D11_TEXTURE2D_DESC dsd{};
	dsd.Width = width;
	dsd.Height = height;
	dsd.MipLevels = 1;
	dsd.ArraySize = 1;
	dsd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsd.SampleDesc.Count = 1;
	dsd.Usage = D3D11_USAGE_DEFAULT;
	dsd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	d3d->CreateTexture2D(&dsd, nullptr, &depth_stencil);
	D3D11_DEPTH_STENCIL_VIEW_DESC vd{};
	vd.Format = dsd.Format;
	vd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	vd.Texture2D.MipSlice = 0;
	d3d->CreateDepthStencilView(depth_stencil.Get(), &vd, &dsv);
	buffer.As(&dxgi);
	d2d_ctx->CreateBitmapFromDxgiSurface(dxgi.Get(), D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW, D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)), &surface);
	d2d_ctx->SetTarget(surface.Get());
	this->width = width;
	this->height = height;
}

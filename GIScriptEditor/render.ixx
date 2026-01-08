module;
#include <d3d11_4.h>
#include <dcomp.h>
#include <dwrite_3.h>
#include <wrl.h>
export module render;

import std;

using namespace Microsoft::WRL;

export namespace Editor::Render
{
	struct RenderStyle
	{
		ComPtr<ID2D1Brush> brush;
		ComPtr<ID2D1StrokeStyle> stroke;
		ComPtr<ID2D1Effect> effect;
		float stroke_width;
		bool need_transform;

		ID2D1Brush* Brush(const D2D1_RECT_F& rect) const
		{
			if (need_transform) brush->SetTransform(D2D1::Matrix3x2F::Scale(rect.right - rect.left, rect.bottom - rect.top) * D2D1::Matrix3x2F::Translation(rect.left, rect.top));
			return brush.Get();
		}

		ID2D1StrokeStyle* Stroke() const { return stroke.Get(); }
		ID2D1Effect* Effect() const { return effect.Get(); }
	};

	class StyleBuilder
	{
		ID2D1DeviceContext* ctx;
		ComPtr<ID2D1Factory> factory;
		RenderStyle style;
	public:
		explicit StyleBuilder(ID2D1DeviceContext* ctx) : ctx(ctx), style()
		{
			ctx->GetFactory(&factory);
		}

		StyleBuilder& Color(float r, float g, float b, float a)
		{
			ComPtr<ID2D1SolidColorBrush> br;
			ctx->CreateSolidColorBrush({ r,g,b,a }, &br);
			style.brush = std::move(br);
			style.need_transform = false;
			return *this;
		}

		StyleBuilder& LinearGradient(const std::vector<D2D1_GRADIENT_STOP>& gss, D2D1_POINT_2F sp, D2D1_POINT_2F ep)
		{
			ComPtr<ID2D1LinearGradientBrush> br;
			ComPtr<ID2D1GradientStopCollection> gs;
			ctx->CreateGradientStopCollection(gss.data(), gss.size(), D2D1_GAMMA_2_2, D2D1_EXTEND_MODE_CLAMP, &gs);
			ctx->CreateLinearGradientBrush({ sp,ep }, D2D1::BrushProperties(), gs.Get(), &br);
			style.brush = std::move(br);
			style.need_transform = true;
			return *this;
		}

		StyleBuilder& Line(float width = 1)
		{
			ComPtr<ID2D1StrokeStyle> st;
			auto sp = D2D1::StrokeStyleProperties(
				D2D1_CAP_STYLE_FLAT,
				D2D1_CAP_STYLE_FLAT,
				D2D1_CAP_STYLE_FLAT,
				D2D1_LINE_JOIN_MITER,
				10.0f,
				D2D1_DASH_STYLE_SOLID,
				0.0f
			);
			factory->CreateStrokeStyle(sp, nullptr, 0, &st);
			style.stroke = std::move(st);
			style.stroke_width = width;
			return *this;
		}

		StyleBuilder& Dilate(UINT width = 1)
		{
			ComPtr<ID2D1Effect> effect;
			ctx->CreateEffect(CLSID_D2D1Morphology, &effect);
			effect->SetValue(D2D1_MORPHOLOGY_PROP_MODE, D2D1_MORPHOLOGY_MODE_DILATE);
			effect->SetValue(D2D1_MORPHOLOGY_PROP_WIDTH, width);
			effect->SetValue(D2D1_MORPHOLOGY_PROP_HEIGHT, width);
			style.effect = std::move(effect);
			return *this;
		}

		RenderStyle Build() { return std::move(style); }
	};

	class Renderer
	{
		ComPtr<ID3D11Device> d3d;
		ComPtr<ID3D11DeviceContext> d3d_ctx;
		ComPtr<ID3D11Texture2D> depth_stencil;
		ComPtr<ID3D11RenderTargetView> rtv;
		ComPtr<ID3D11DepthStencilView> dsv;
		ComPtr<ID2D1Device> d2d;
		ComPtr<ID2D1DeviceContext> d2d_ctx;
		ComPtr<ID2D1Bitmap1> surface;
		ComPtr<IDXGISwapChain4> swap_chain;
		ComPtr<IDCompositionDesktopDevice> dcomp;
		ComPtr<IDCompositionTarget> target;
		ComPtr<IDWriteFactory> dwrite;
		ComPtr<IDWriteTextFormat> default_format;
		HANDLE swap_event;
		ComPtr<ID3D11BlendState> blend;
		float blend_factor[4]{};
		float background[4];
		float width, height;
	public:
		Renderer();
		void BeginRender();
		void EndRender() const;
		void Bind(HWND window);
		void Resize(int width, int height);

		ID2D1DeviceContext* D2DCtx() const { return d2d_ctx.Get(); }
		IDWriteFactory* DWrite() const { return dwrite.Get(); }
		IDWriteTextFormat* DefaultFormat() const { return default_format.Get(); }
		StyleBuilder Style() const { return StyleBuilder{ D2DCtx() }; }
		float Width() const { return width; }
		float Height() const { return height; }

		ComPtr<ID2D1PathGeometry> PathGeometry() const
		{
			ComPtr<ID2D1PathGeometry> pg;
			ComPtr<ID2D1Factory> factory;
			d2d_ctx->GetFactory(&factory);
			factory->CreatePathGeometry(&pg);
			return pg;
		}
	};

	struct RenderAble
	{
		virtual void Render(const Renderer& renderer) = 0;
		virtual ~RenderAble() = default;
	};
};

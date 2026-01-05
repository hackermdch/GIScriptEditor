module;
#include <d2d1_3.h>
#include <dwrite_3.h>
#include <wrl/client.h>
export module widgets;

import std;
import render;

using namespace Microsoft::WRL;

export namespace Editor::UI
{
	enum class Anchor
	{
		LeftTop,
		Top,
		RightTop,
		Left,
		Center,
		Right,
		LeftBottom,
		Bottom,
		RightBottom
	};

	class Widget : public Render::RenderAble
	{
	protected:
		bool interactive;

		float CalcX(const Render::Renderer& renderer) const;
		float CalcY(const Render::Renderer& renderer) const;
	public:
		float x, y;
		float width, height;
		Anchor anchor;

		struct
		{
			float x, y;
		} origin;

		explicit Widget(float x = 0, float y = 0, float width = 100, float height = 60, Anchor anchor = {});
		Widget(Widget&&) = default;
		void SetOrigin(float x, float y);
		void SetOriginCenter();
		D2D1_POINT_2F TransformLocal(const Render::Renderer& renderer, float x, float y) const;
		bool HitTest(const Render::Renderer& renderer, float mx, float my) const;
		virtual void SetStyle(Render::RenderStyle style, int slot = 0) = 0;
		virtual void OnMouseEnter() {}
		virtual void OnMouseLeave() {}
		virtual void OnMouseMove(float x, float y) {}
		virtual void OnMouseClick(float x, float y) {}
	};

	class Button : public Widget
	{
		Render::RenderStyle body, border, hovered_body, hovered_border, text_s;
		ComPtr<IDWriteTextLayout> text;
		bool hovered = false;
		std::function<void(Button& button)> clicked;
	public:
		explicit Button(const Render::Renderer& renderer, const std::wstring& text = L"", float width = 120, float height = 60);
		void Render(const Render::Renderer& renderer) override;
		void SetText(const Render::Renderer& renderer, const std::wstring& text, IDWriteTextFormat* format = nullptr);
		void SetTextSize(float size) const;
		void SetStyle(Render::RenderStyle style, int slot = 0) override;
		void OnMouseEnter() override;
		void OnMouseLeave() override;
		void OnMouseClick(float x, float y) override;

		void SetClickEvent(decltype(clicked) event) { clicked = std::move(event); }
	};

	class TextBox : public Widget
	{
		Render::RenderStyle fill, stroke;
		ComPtr<IDWriteTextLayout> text;
		ComPtr<ID2D1BitmapRenderTarget> layer;
		bool dirty = true;
	public:
		TextBox(const Render::Renderer& renderer, const std::wstring& text, float width = 120, float height = 60);
		void Render(const Render::Renderer& renderer) override;
		void SetText(const Render::Renderer& renderer, const std::wstring& text, IDWriteTextFormat* format = nullptr);
		void SetTextSize(float size);
		void SetStyle(Render::RenderStyle style, int slot = 0) override;
	};

	class ImageButton : public Widget
	{
		ComPtr<ID2D1Image> source;
		Render::RenderStyle overlayer;
		bool hovered = false;
		std::function<void(ImageButton& button)> clicked;
	public:
		explicit ImageButton(const Render::Renderer& renderer, ComPtr<ID2D1Image> image, float width = 60, float height = 60);
		void Render(const Render::Renderer& renderer) override;
		void SetStyle(Render::RenderStyle style, int slot) override;
		void OnMouseEnter() override;
		void OnMouseLeave() override;
		void OnMouseClick(float x, float y) override;

		void SetClickEvent(decltype(clicked) event) { clicked = std::move(event); }
	};

	class StaticShape : public Widget
	{
	protected:
		Render::RenderStyle fill, stroke;
	public:
		StaticShape(float width, float height);
		void SetStyle(Render::RenderStyle style, int slot) override;
	};

	class Rectangle : public StaticShape
	{
	public:
		explicit Rectangle(const Render::Renderer& renderer, float width = 120, float height = 60);
		void Render(const Render::Renderer& renderer) override;
	};
}
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

	class Widget;

	class IWidgetContainer
	{
		friend Widget;
		virtual float ICalcX(const Widget& w) = 0;
		virtual float ICalcY(const Widget& w) = 0;
	public:
		virtual ~IWidgetContainer() = 0;
	};

	template<typename E> requires std::is_base_of_v<Widget, E>
	class WidgetContainer : public IWidgetContainer
	{
		friend Widget;

		float ICalcX(const Widget& w) final { return CalcX((const E&)w); }
		float ICalcY(const Widget& w) final { return CalcY((const E&)w); }
	protected:
		std::deque<std::unique_ptr<E>> widgets;

		virtual float CalcX(const E& w) = 0;
		virtual float CalcY(const E& w) = 0;
	public:
		template<typename T> requires std::is_base_of_v<E, T>
		T& AddWidget(std::unique_ptr<T> widget)
		{
			widget->container = this;
			return (T&)*widgets.emplace_back(std::move(widget));
		}
	};

	class RelativeLayout : public WidgetContainer<Widget>
	{
		std::function<float()> width, height;

		float CalcX(const Widget& w) override;
		float CalcY(const Widget& w) override;
	public:
		RelativeLayout(decltype(width) width, decltype(height) height) : width(std::move(width)), height(std::move(height))
		{
		}
	};

	class Widget : public Render::RenderAble
	{
		friend WidgetContainer;
		IWidgetContainer* container = nullptr;
	protected:
		bool interactive;

		float CalcX() const;
		float CalcY() const;
	public:
		float x, y;
		float width, height;
		Anchor anchor;
		bool visible;

		struct
		{
			float x, y;
		} origin;

		explicit Widget(float x = 0, float y = 0, float width = 100, float height = 60, Anchor anchor = {});
		Widget(Widget&&) = default;
		void SetOrigin(float x, float y);
		void SetOriginCenter();
		D2D1_POINT_2F TransformLocal(float x, float y) const;
		bool HitTest(float cx, float cy) const;
		virtual void SetStyle(Render::RenderStyle style, int slot = 0) = 0;
		virtual void OnMouseEnter() {}
		virtual void OnMouseLeave() {}
		virtual void OnMouseMove(float x, float y) {}
		virtual void OnMouseClick(float x, float y) {}
		virtual void OnMouseWheel(float x, float y, float delta) {}
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

	class PopMenu : public Widget
	{
		std::vector<ComPtr<IDWriteTextLayout>> items;
		ComPtr<IDWriteTextLayout> title;
		Render::RenderStyle background, fill, stroke, highlight;
		float item_width, item_height;
		int hovered = -1;
		std::function<void(PopMenu& menu, int index)> clicked;
	public:
		PopMenu(const Render::Renderer& renderer, const std::vector<std::wstring>& items, float item_width, float item_height);
		void Render(const Render::Renderer& renderer) override;
		void OnMouseLeave() override;
		void OnMouseMove(float x, float y) override;
		void OnMouseClick(float x, float y) override;
		void SetTextSize(float size) const;
		void SetStyle(Render::RenderStyle style, int slot) override;
		void SetTitle(const Render::Renderer& renderer, const std::wstring& text, float size);

		void SetClickEvent(decltype(clicked) event) { clicked = std::move(event); }
	};

	class ListViewItem : public Widget
	{
	public:
		ListViewItem(float width, float height) :Widget(0, 0, width, height) {}
		virtual bool Highlight() { return true; }
	};

	class ListView : public Widget, public WidgetContainer<ListViewItem>
	{
		Render::RenderStyle background, fill, stroke, highlight;
		Widget* hovered = nullptr;
		float item_offset_x = 0;
		float item_offset_y = 0;
		float scroll_y = 0;
		float max_scroll_y = 0;

		float CalcX(const ListViewItem& w) override;
		float CalcY(const ListViewItem& w) override;
	public:
		ListView(const Render::Renderer& renderer, float width, float height);
		void OnMouseLeave() override;
		void OnMouseMove(float x, float y) override;
		void OnMouseClick(float x, float y) override;
		void OnMouseWheel(float x, float y, float delta) override;
		void Render(const Render::Renderer& renderer) override;
		void SetStyle(Render::RenderStyle style, int slot) override;
		void Clear();
	};
}
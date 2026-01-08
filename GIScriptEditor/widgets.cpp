module;
#include <d2d1_3.h>
#include <dwrite_3.h>
module widgets;

using namespace Editor::UI;
using namespace Editor::Render;

IWidgetContainer::~IWidgetContainer()
{
}

float RelativeLayout::CalcX(const Widget& w)
{
	switch (w.anchor)
	{
	case Anchor::LeftTop:
	case Anchor::Left:
	case Anchor::LeftBottom:
		return w.x - w.origin.x;
	case Anchor::Top:
	case Anchor::Center:
	case Anchor::Bottom:
		return w.x - w.origin.x + width() / 2;
	case Anchor::RightTop:
	case Anchor::Right:
	case Anchor::RightBottom:
		return w.x - w.origin.x + width();
	}
	return 0;
}

float RelativeLayout::CalcY(const Widget& w)
{
	switch (w.anchor)
	{
	case Anchor::LeftTop:
	case Anchor::Top:
	case Anchor::RightTop:
		return w.y - w.origin.y;
	case Anchor::Left:
	case Anchor::Center:
	case Anchor::Right:
		return w.y - w.origin.y + height() / 2;
	case Anchor::LeftBottom:
	case Anchor::Bottom:
	case Anchor::RightBottom:
		return w.y - w.origin.y + height();
	}
	return 0;
}

float Widget::CalcX() const
{
	return container->ICalcX(*this);
}

float Widget::CalcY() const
{
	return container->ICalcY(*this);
}

Widget::Widget(float x, float y, float width, float height, Anchor anchor) : interactive(true), x(x), y(y), width(width), height(height), anchor(anchor), visible(true), origin()
{
}

void Widget::SetOrigin(float x, float y)
{
	origin.x = x;
	origin.y = y;
}

void Widget::SetOriginCenter()
{
	origin.x = width / 2;
	origin.y = height / 2;
}

D2D1_POINT_2F Widget::TransformLocal(float x, float y) const
{
	auto ox = CalcX(), oy = CalcY();
	return { x - ox ,y - oy };
}

bool Widget::HitTest(float cx, float cy) const
{
	if (!interactive) return false;
	auto x = CalcX(), y = CalcY();
	D2D1_RECT_F rect{ x,y,x + width,y + height };
	return rect.left <= cx && cx <= rect.right && rect.top <= cy && cy <= rect.bottom;
}

Button::Button(const Renderer& renderer, const std::wstring& text, float width, float height) : Widget(0, 0, width, height)
{
	body = renderer.Style().Color(0.8, 0.8, 0.8, 1).Build();
	border = renderer.Style().Color(0.3, 0.3, 0.3, 1).Line(3).Build();
	hovered_body = renderer.Style().LinearGradient({ {0,0.4,1,1,1},{0.3,1,1,1,1},{0.7,1,1,1,1},{1,0.4,1,1,1} }, {}, { 1,1 }).Build();
	hovered_border = renderer.Style().Color(0.7, 0.6, 0.5, 1).Line(3).Build();
	text_s = renderer.Style().Color(0, 0, 0, 1).Build();
	SetText(renderer, text);
}

void Button::Render(const Renderer& renderer)
{
	auto ctx = renderer.D2DCtx();
	auto x = CalcX(), y = CalcY();
	D2D1_RECT_F collider{ x,y,x + width,y + height };
	D2D1_ROUNDED_RECT rect{ x ,y,x + width,y + height,3,3 };
	DWRITE_TEXT_METRICS metrics;
	text->GetMetrics(&metrics);
	D2D1_POINT_2F p{ x + (width - metrics.width) / 2, y + (height - metrics.height) / 2 };
	if (hovered)
	{
		ctx->FillRoundedRectangle(rect, hovered_body.Brush(collider));
		ctx->DrawTextLayout(p, text.Get(), text_s.Brush(collider));
		ctx->DrawRoundedRectangle(rect, hovered_border.Brush(collider), hovered_border.stroke_width, hovered_border.Stroke());
	}
	else
	{
		ctx->FillRoundedRectangle(rect, body.Brush(collider));
		ctx->DrawTextLayout(p, text.Get(), text_s.Brush(collider));
		ctx->DrawRoundedRectangle(rect, border.Brush(collider), border.stroke_width, border.Stroke());
	}
}

void Button::SetText(const Renderer& renderer, const std::wstring& text, IDWriteTextFormat* format)
{
	renderer.DWrite()->CreateTextLayout(text.data(), text.size(), format ? format : renderer.DefaultFormat(), width, height, &this->text);
}

void Button::SetTextSize(float size) const
{
	text->SetFontSize(size, { 0,~0u });
	text->SetMaxWidth(width);
	text->SetMaxHeight(height);
}

void Button::OnMouseEnter()
{
	hovered = true;
}

void Button::OnMouseLeave()
{
	hovered = false;
}

void Button::OnMouseClick(float x, float y)
{
	if (clicked) clicked(*this);
}

void Button::SetStyle(RenderStyle style, int slot)
{
	switch (slot)
	{
	case 0:
		body = std::move(style);
		break;
	case 1:
		border = std::move(style);
		break;
	case 2:
		hovered_body = std::move(style);
		break;
	case 3:
		hovered_border = std::move(style);
		break;
	case 4:
		text_s = std::move(style);
		break;
	}
}

TextBox::TextBox(const Renderer& renderer, const std::wstring& text, float width, float height) : Widget(0, 0, width, height)
{
	interactive = false;
	fill = renderer.Style().Color(0.9, 0.9, 0.9, 1).Build();
	stroke = renderer.Style().Color(0, 0, 0, 1).Dilate(4).Build();
	SetText(renderer, text);
}

static void DrawTextOutline(ID2D1DeviceContext* ctx, ID2D1BitmapRenderTarget* layer, IDWriteTextLayout* text, D2D1_POINT_2F pos, ID2D1Brush* brush, ID2D1Effect* effect, bool update)
{
	ComPtr<ID2D1Bitmap> bitmap;
	if (update)
	{
		layer->BeginDraw();
		layer->Clear();
		layer->DrawTextLayout({}, text, brush);
		layer->EndDraw();
	}
	layer->GetBitmap(&bitmap);
	effect->SetInput(0, bitmap.Get());
	ctx->DrawImage(effect, pos);
}

void TextBox::Render(const Renderer& renderer)
{
	auto ctx = renderer.D2DCtx();
	auto x = CalcX(), y = CalcY();
	D2D1_RECT_F collider{ x,y,x + width,y + height };
	if (dirty)
	{
		DWRITE_TEXT_METRICS metrics;
		text->GetMetrics(&metrics);
		ctx->CreateCompatibleRenderTarget({ metrics.width,metrics.height }, &layer);
		DrawTextOutline(ctx, layer.Get(), text.Get(), { x,y }, stroke.Brush(collider), stroke.Effect(), true);
		dirty = false;
	}
	else DrawTextOutline(ctx, layer.Get(), text.Get(), { x,y }, stroke.Brush(collider), stroke.Effect(), false);
	ctx->DrawTextLayout({ x,y }, text.Get(), fill.Brush(collider));
}

void TextBox::SetText(const Renderer& renderer, const std::wstring& text, IDWriteTextFormat* format)
{
	renderer.DWrite()->CreateTextLayout(text.data(), text.size(), format ? format : renderer.DefaultFormat(), width, height, &this->text);
	dirty = true;
}

void TextBox::SetTextSize(float size)
{
	text->SetFontSize(size, { 0,~0u });
	text->SetMaxWidth(width);
	text->SetMaxHeight(height);
	dirty = true;
}

void TextBox::SetStyle(RenderStyle style, int slot)
{
	switch (slot)
	{
	case 0:
		fill = std::move(style);
		break;
	case 1:
		stroke = std::move(style);
		break;
	}
}

ImageButton::ImageButton(const Renderer& renderer, ComPtr<ID2D1Image> image, float width, float height) : Widget(0, 0, width, height), source(std::move(image))
{
	overlayer = renderer.Style().Color(0.8, 0.8, 0.8, 0.8).Build();
}

void ImageButton::Render(const Renderer& renderer)
{
	auto ctx = renderer.D2DCtx();
	auto x = CalcX(), y = CalcY();
	ctx->DrawImage(source.Get(), { x,y });
	if (hovered) ctx->FillRectangle({ x,y,x + width,y + height }, overlayer.Brush({ x,y,width,height }));
}

void ImageButton::OnMouseEnter()
{
	hovered = true;
}

void ImageButton::OnMouseLeave()
{
	hovered = false;
}

void ImageButton::OnMouseClick(float x, float y)
{
	if (clicked) clicked(*this);
}

void ImageButton::SetStyle(RenderStyle style, int slot)
{
	if (slot == 0) overlayer = std::move(style);
}

StaticShape::StaticShape(float width, float height) : Widget(0, 0, width, height), fill(), stroke()
{
	interactive = false;
}

void StaticShape::SetStyle(RenderStyle style, int slot)
{
	switch (slot)
	{
	case 0:
		fill = std::move(style);
		break;
	case 1:
		stroke = std::move(style);
		break;
	}
}

Rectangle::Rectangle(const Renderer& renderer, float width, float height) : StaticShape(width, height)
{
	fill = renderer.Style().Color(0.8, 0.8, 0.8, 0.6).Build();
	stroke = renderer.Style().Color(0.3, 0.3, 0.3, 1).Line(3).Build();
}

void Rectangle::Render(const Renderer& renderer)
{
	auto ctx = renderer.D2DCtx();
	auto x = CalcX(), y = CalcY();
	D2D1_RECT_F rect{ x,y,x + width,y + height };
	ctx->FillRectangle(rect, fill.Brush(rect));
	ctx->DrawRectangle(rect, stroke.Brush(rect), stroke.stroke_width, stroke.Stroke());
}

static std::vector<ComPtr<IDWriteTextLayout>> MakePopItems(const Renderer& renderer, const std::vector<std::wstring>& items)
{
	std::vector<ComPtr<IDWriteTextLayout>> ts;
	for (auto& s : items) renderer.DWrite()->CreateTextLayout(s.data(), s.size(), renderer.DefaultFormat(), 4600, 40, &ts.emplace_back());
	return ts;
}

PopMenu::PopMenu(const Renderer& renderer, const std::vector<std::wstring>& items, float item_width, float item_height) : Widget(0, 0, item_width, item_height* items.size()), items(MakePopItems(renderer, items)), item_width(item_width), item_height(item_height)
{
	visible = false;
	background = renderer.Style().Color(0.8, 0.8, 0.8, 0.6).Build();
	fill = renderer.Style().Color(0, 0, 0, 1).Build();
	stroke = renderer.Style().Color(0.3, 0.3, 0.3, 1).Line(3).Build();
	highlight = renderer.Style().Color(0.6, 1, 1, 1).Build();
}

void PopMenu::Render(const Renderer& renderer)
{
	auto ctx = renderer.D2DCtx();
	auto x = CalcX(), y = CalcY();
	auto y1 = y;
	int i = 0;
	if (title)
	{
		DWRITE_TEXT_METRICS metrics;
		title->GetMetrics(&metrics);
		D2D1_RECT_F collider{ x,y - metrics.height,x + item_width,y };
		D2D1_POINT_2F p{ x + (item_width - metrics.width) / 2, y - metrics.height };
		ctx->DrawTextLayout(p, title.Get(), fill.Brush(collider));
	}
	for (auto& it : items)
	{
		D2D1_RECT_F rect{ x,y1,x + item_width,y1 + item_height };
		ctx->FillRectangle(rect, i == hovered ? highlight.Brush(rect) : background.Brush(rect));
		ctx->DrawRectangle(rect, stroke.Brush(rect), stroke.stroke_width, stroke.Stroke());
		DWRITE_TEXT_METRICS metrics;
		it->GetMetrics(&metrics);
		D2D1_POINT_2F p{ x + (item_width - metrics.width) / 2, y1 + (item_height - metrics.height) / 2 };
		ctx->DrawTextLayout(p, it.Get(), fill.Brush(rect));
		y1 += item_height;
		i++;
	}
}

void PopMenu::OnMouseLeave()
{
	hovered = -1;
}

void PopMenu::OnMouseMove(float x, float y)
{
	hovered = y / item_height;
}

void PopMenu::OnMouseClick(float x, float y)
{
	if (!clicked) return;
	hovered = y / item_height;
	if (hovered >= 0 && hovered < items.size()) clicked(*this, hovered);
}

void PopMenu::SetTextSize(float size) const
{
	for (auto& i : items) i->SetFontSize(size, { 0,~0u });
	for (auto& i : items) i->SetFontWeight(DWRITE_FONT_WEIGHT_BOLD, { 0,~0u });
}

void PopMenu::SetStyle(RenderStyle style, int slot)
{
	switch (slot)
	{
	case 0:
		background = std::move(style);
		break;
	case 1:
		fill = std::move(style);
		break;
	case 2:
		stroke = std::move(style);
		break;
	case 3:
		highlight = std::move(style);
		break;
	}
}

void PopMenu::SetTitle(const Renderer& renderer, const std::wstring& text, float size)
{
	renderer.DWrite()->CreateTextLayout(text.data(), text.size(), renderer.DefaultFormat(), 4600, 40, &title);
	title->SetFontSize(size, { 0,~0u });
}

float ListView::CalcX(const ListViewItem& w)
{
	return item_offset_x;
}

float ListView::CalcY(const ListViewItem& w)
{
	return item_offset_y;
}

ListView::ListView(const Renderer& renderer, float width, float height) : Widget(0, 0, width, height)
{
	background = renderer.Style().Color(0.8, 0.8, 0.8, 0.6).Build();
	fill = renderer.Style().Color(0, 0, 0, 1).Build();
	stroke = renderer.Style().Color(0.3, 0.3, 0.3, 1).Line(3).Build();
	highlight = renderer.Style().Color(0.6, 1, 1, 0.5).Build();
}

void ListView::OnMouseLeave()
{
	if (!hovered) return;
	hovered->OnMouseLeave();
	hovered = nullptr;
}

void ListView::OnMouseMove(float x, float y)
{
	auto old = hovered;
	auto hit = true;
	item_offset_x = 0;
	item_offset_y = scroll_y;
	for (auto& w : widgets)
	{
		if (hit && w->visible && w->HitTest(x, y))
		{
			hit = false;
			if (hovered != w.get())
			{
				hovered = w.get();
				w->OnMouseEnter();
			}
			auto [px, py] = w->TransformLocal(x, y);
			w->OnMouseMove(px, py);
		}
		else if (old == w.get()) w->OnMouseLeave();
		item_offset_y += w->height;
	}
	if (hit) hovered = nullptr;
}

void ListView::OnMouseClick(float x, float y)
{
	if (!hovered) return;
	auto [px, py] = hovered->TransformLocal(x, y);
	hovered->OnMouseClick(px, py);
}

void ListView::OnMouseWheel(float x, float y, float delta)
{
	scroll_y += delta * 20;
}

void ListView::Render(const Renderer& renderer)
{
	item_offset_y = 0;
	for (auto& w : widgets) item_offset_y += w->height;
	max_scroll_y = item_offset_y > height ? item_offset_y - height : 0;
	if (scroll_y > 0) scroll_y -= scroll_y / 20;
	if (scroll_y < -max_scroll_y) scroll_y -= (max_scroll_y + scroll_y) / 20;
	auto ctx = renderer.D2DCtx();
	auto x = Widget::CalcX(), y = Widget::CalcY();
	item_offset_x = x;
	item_offset_y = y + scroll_y;
	D2D1_RECT_F rect{ x,y,x + width,y + height };
	ctx->FillRectangle(rect, background.Brush(rect));
	ctx->DrawRectangle(rect, stroke.Brush(rect), stroke.stroke_width, stroke.Stroke());
	rect.top += 2;
	rect.bottom -= 2;
	ctx->PushAxisAlignedClip(rect, D2D1_ANTIALIAS_MODE_ALIASED);
	for (auto& w : widgets)
	{
		auto bottom = item_offset_y + w->height;
		if (bottom > y + height) bottom = y + height;
		D2D1_RECT_F clip{ x,item_offset_y,x + width, bottom };
		ctx->PushAxisAlignedClip(clip, D2D1_ANTIALIAS_MODE_ALIASED);
		w->Render(renderer);
		if (w.get() == hovered && w->Highlight()) ctx->FillRectangle(clip, highlight.Brush(clip));
		ctx->PopAxisAlignedClip();
		item_offset_y += w->height;
	}
	ctx->PopAxisAlignedClip();
}

void ListView::SetStyle(RenderStyle style, int slot)
{
	switch (slot)
	{
	case 0:
		background = std::move(style);
		break;
	case 1:
		fill = std::move(style);
		break;
	case 2:
		stroke = std::move(style);
		break;
	case 3:
		highlight = std::move(style);
		break;
	}
}

void ListView::Clear()
{
	widgets.clear();
}

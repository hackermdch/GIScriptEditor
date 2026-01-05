module;
#include <d2d1_3.h>
#include <dwrite_3.h>
module widgets;

using namespace Editor::UI;
using namespace Editor::Render;

float Widget::CalcX(const Renderer& renderer) const
{
	switch (anchor)
	{
	case Anchor::LeftTop:
	case Anchor::Left:
	case Anchor::LeftBottom:
		return x - origin.x;
	case Anchor::Top:
	case Anchor::Center:
	case Anchor::Bottom:
		return x - origin.x + renderer.Width() / 2;
	case Anchor::RightTop:
	case Anchor::Right:
	case Anchor::RightBottom:
		return x - origin.x + renderer.Width();
	}
	return 0;
}

float Widget::CalcY(const Renderer& renderer) const
{
	switch (anchor)
	{
	case Anchor::LeftTop:
	case Anchor::Top:
	case Anchor::RightTop:
		return y - origin.y;
	case Anchor::Left:
	case Anchor::Center:
	case Anchor::Right:
		return y - origin.y + renderer.Height() / 2;
	case Anchor::LeftBottom:
	case Anchor::Bottom:
	case Anchor::RightBottom:
		return y - origin.y + renderer.Height();
	}
	return 0;
}

Widget::Widget(float x, float y, float width, float height, Anchor anchor) : interactive(true), x(x), y(y), width(width), height(height), anchor(anchor), origin()
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

D2D1_POINT_2F Widget::TransformLocal(const Renderer& renderer, float x, float y) const
{
	auto ox = CalcX(renderer), oy = CalcY(renderer);
	return { x - ox ,y - oy };
}

bool Widget::HitTest(const Renderer& renderer, float mx, float my) const
{
	if (!interactive) return false;
	auto x = CalcX(renderer), y = CalcY(renderer);
	D2D1_RECT_F rect{ x,y,x + width,y + height };
	return rect.left <= mx && mx <= rect.right && rect.top <= my && my <= rect.bottom;
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
	auto x = CalcX(renderer), y = CalcY(renderer);
	D2D1_RECT_F collider{ x,y,width,height };
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
	auto x = CalcX(renderer), y = CalcY(renderer);
	D2D1_RECT_F collider{ x,y,width,height };
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
	auto x = CalcX(renderer), y = CalcY(renderer);
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
	auto x = CalcX(renderer), y = CalcY(renderer);
	D2D1_RECT_F collider{ x,y,width,height }, rect{ x,y,x + width,y + height };
	ctx->FillRectangle(rect, fill.Brush(collider));
	ctx->DrawRectangle(rect, stroke.Brush(collider), stroke.stroke_width, stroke.Stroke());
}

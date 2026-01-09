module;
#include <GINodeGraph.h>
#include <d2d1_3.h>
#include <dwrite_3.h>
module app.widgets;

import util;

using namespace Ugc::NodeGraph;
using namespace Editor;
using namespace UI;
using namespace App;

static D2D1::Matrix3x2F Transform(float x, float y, float width, float height)
{
	return D2D1::Matrix3x2F::Scale(width, height) * D2D1::Matrix3x2F::Translation(x, y);
}

NodeReferenceItem::NodeReferenceItem(const Render::Renderer& renderer, NodeReference& reference, float width) : ListViewItem(width, 40)
{
	auto ws = Utils::ToUtf16(reference.name);
	renderer.DWrite()->CreateTextLayout(ws.data(), ws.size(), renderer.DefaultFormat(), 4600, 40, &name);
	for (auto& [name, composite, ids] : reference.referenced)
	{
		auto& r = referenced.emplace_back();
		auto str = Utils::ToUtf16(name), str2 = std::to_wstring(ids.size());
		renderer.DWrite()->CreateTextLayout(str.data(), str.size(), renderer.DefaultFormat(), 4600, 40, &r.name);
		renderer.DWrite()->CreateTextLayout(str2.data(), str2.size(), renderer.DefaultFormat(), 120, 40, &r.count);
		r.name->SetFontSize(26, { 0,~0u });
		r.count->SetFontSize(26, { 0,~0u });
		r.composite = composite;
		r.referenced_ids = std::move(ids);
	}
	auto c = 0;
	for (auto& r : referenced) c += r.referenced_ids.size();
	ws = std::to_wstring(c);
	renderer.DWrite()->CreateTextLayout(ws.data(), ws.size(), renderer.DefaultFormat(), 120, 40, &count);
	name->SetFontSize(26, { 0,~0u });
	count->SetFontSize(26, { 0,~0u });
	expand_height = (referenced.size() + 1) * 40;
	normal = renderer.Style().Color(0.6, 1, 1, 1).Build();
	composite = renderer.Style().Color(0.8, 0, 0.8, 1).Build();
	misc = renderer.Style().Color(0.5, 1, 0.5, 1).Build();
	button_fill = renderer.Style().Color(0.8, 0.8, 0.8, 1).Build();
	button_stroke = renderer.Style().Color(0.3, 0.3, 0.3, 1).Build();
	button_highlight = renderer.Style().Color(0.8, 0.8, 0.3, 1).Build();
	other = renderer.Style().Color(1, 1, 0.5, 1).Build();
	button1 = renderer.PathGeometry();
	button2 = renderer.PathGeometry();
	ComPtr<ID2D1GeometrySink> sink;
	button1->Open(&sink);
	sink->BeginFigure({ 0,0 }, D2D1_FIGURE_BEGIN_FILLED);
	sink->AddLine({ 1,0 });
	sink->AddLine({ 0.5,1 });
	sink->EndFigure(D2D1_FIGURE_END_CLOSED);
	sink->Close();
	button2->Open(&sink);
	sink->BeginFigure({ 0.5,0 }, D2D1_FIGURE_BEGIN_FILLED);
	sink->AddLine({ 0,1 });
	sink->AddLine({ 1,1 });
	sink->EndFigure(D2D1_FIGURE_END_CLOSED);
	sink->Close();
}

void NodeReferenceItem::OnMouseLeave()
{
	button_hovered = false;
}

void NodeReferenceItem::OnMouseMove(float x, float y)
{
	if (referenced.empty()) return;
	D2D1_RECT_F rect{ width - 60, 5, width - 30, 35 };
	button_hovered = rect.left <= x && x <= rect.right && rect.top <= y && y <= rect.bottom;
}

void NodeReferenceItem::OnMouseClick(float x, float y)
{
	if (button_hovered)
	{
		if (expand)
		{
			expand = false;
			height = 40;
		}
		else
		{
			expand = true;
			height = expand_height;
		}
	}
}

void NodeReferenceItem::Render(const Render::Renderer& renderer)
{
	auto ctx = renderer.D2DCtx();
	auto x = CalcX(), y = CalcY();
	D2D1_RECT_F collider{ x,y,x + width, y + 40 };
	ctx->DrawTextLayout({ x + 5,y }, name.Get(), composite.Brush(collider));
	{
		DWRITE_TEXT_METRICS metrics;
		count->GetMetrics(&metrics);
		ctx->DrawTextLayout({ x + width - 120 - metrics.width,y }, count.Get(), misc.Brush(collider));
	}
	if (!referenced.empty())
	{
		D2D1_MATRIX_3X2_F old;
		auto trans = Transform(x + width - 60, y + 5, 30, 30);
		auto trans2 = Transform(x + width - 61, y + 4, 32, 32);
		ctx->GetTransform(&old);
		ctx->SetTransform(trans2);
		auto br = button_hovered ? button_highlight.brush.Get() : button_fill.brush.Get(), br2 = button_stroke.brush.Get();
		if (button_fill.need_transform) br->SetTransform(trans);
		if (button_stroke.need_transform) br2->SetTransform(trans2);
		if (expand)
		{
			ctx->FillGeometry(button2.Get(), br2);
			ctx->SetTransform(trans);
			ctx->FillGeometry(button2.Get(), br);
		}
		else
		{
			ctx->FillGeometry(button1.Get(), br2);
			ctx->SetTransform(trans);
			ctx->FillGeometry(button1.Get(), br);
		}
		ctx->SetTransform(old);
	}
	if (!expand) return;
	auto y1 = y + 40;
	for (auto& r : referenced)
	{
		collider.top = y1;
		collider.bottom = y1 + 40;
		DWRITE_TEXT_METRICS metrics;
		r.name->GetMetrics(&metrics);
		if (r.composite) ctx->DrawTextLayout({ x + width - metrics.width - 100,y1 }, r.name.Get(), composite.Brush(collider));
		else ctx->DrawTextLayout({ x + width - metrics.width - 100,y1 }, r.name.Get(), normal.Brush(collider));
		ctx->DrawTextLayout({ x + width - 60 ,y1 }, r.count.Get(), other.Brush(collider));
		y1 += 40;
	}
}

void NodeReferenceItem::SetStyle(Render::RenderStyle style, int slot)
{
}

bool NodeReferenceItem::Highlight()
{
	return !expand && !button_hovered;
}

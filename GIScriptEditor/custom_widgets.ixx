module;
#include <GINodeGraph.h>
#include <wrl/client.h>
#include <dwrite_3.h>
#include <d2d1_3.h>
export module app.widgets;

import std;
import widgets;
import render;

using namespace Microsoft::WRL;

export namespace Editor::App
{
	class NodeReferenceItem : public UI::ListViewItem
	{
		ComPtr<IDWriteTextLayout> name;
		ComPtr<IDWriteTextLayout> count;
		ComPtr<ID2D1PathGeometry> button1, button2;

		struct Referenced
		{
			ComPtr<IDWriteTextLayout> name;
			bool composite;
		};

		std::vector<Referenced> referenced;
		Render::RenderStyle normal, composite, misc, button_fill, button_stroke, button_highlight;
		float expand_height;
		bool expand = false, button_hovered = false;
	public:
		NodeReferenceItem(const Render::Renderer& renderer, Ugc::NodeGraph::NodeReference& reference, float width);
		void OnMouseLeave() override;
		void OnMouseMove(float x, float y) override;
		void OnMouseClick(float x, float y) override;
		void Render(const Render::Renderer& renderer) override;
		void SetStyle(Render::RenderStyle style, int slot) override;
		bool Highlight() override;
	};
}

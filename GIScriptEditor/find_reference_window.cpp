module;
#include <GINodeGraph.h>
module app.window;

import util;
import app.widgets;

using namespace Editor;
using namespace UI;
using namespace App;

FindReferenceWindow::FindReferenceWindow() : Window(L"查找引用")
{
	{
		auto& b = AddWidget(std::make_unique<Button>(Renderer(), L"打开存档", 160));
		b.anchor = Anchor::Center;
		b.y = 240;
		b.SetOriginCenter();
		b.SetTextSize(26);
		b.SetClickEvent([&](auto&)->Utils::Task
			{
				auto path = co_await OpenFile({ {L"存档文件(*.gil)",L"*.gil"},{L"所有文件(*.*)",L"*.*"} });
				Flush();
				if (path.empty()) co_return;
				list->Clear();
				for (auto& r : Ugc::NodeGraph::LoadProject(path)->GetReferences()) list->AddWidget(std::make_unique<NodeReferenceItem>(Renderer(), r, list->width));
			});
	}
	{
		auto& l = AddWidget(std::make_unique<ListView>(Renderer(), 760, 400));
		l.anchor = Anchor::Center;
		l.y = -50;
		l.SetOriginCenter();
		list = &l;
	}
}
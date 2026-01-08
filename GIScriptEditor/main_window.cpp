module;
#include <Windows.h>
#include <GINodeGraph.h>
#include "resource.h"
module app.window;

import util;
import compiler;
import image;

using namespace Editor;
using namespace UI;
using namespace App;

static auto LoadScript(const std::filesystem::path& path)
{
	std::ifstream in(path);
	std::string code((std::istreambuf_iterator(in)), std::istreambuf_iterator<char>());
	return code;
}

MainWindow::MainWindow() : Window(L"GIScriptEditor")
{
	{
		auto& t = AddWidget(std::make_unique<TextBox>(Renderer(), L"当前版本：1.1.0", 160));
		t.anchor = Anchor::Top;
		t.y = 40;
		t.SetOriginCenter();
		t.SetTextSize(18);
		t.SetStyle(Renderer().Style().Color(0.4, 1, 0.4, 1).Build());
		t.SetStyle(Renderer().Style().Color(1, 1, 0, 1).Dilate().Build(), 1);
	}
	{
		auto& t = AddWidget(std::make_unique<TextBox>(Renderer(), L"存档路径：", 160));
		t.anchor = Anchor::Center;
		t.x = -300;
		t.y = -100;
		t.SetOriginCenter();
		t.SetTextSize(26);
	}
	{
		auto& t = AddWidget(std::make_unique<TextBox>(Renderer(), L"脚本目录：", 160));
		t.anchor = Anchor::Center;
		t.x = -300;
		t.SetOriginCenter();
		t.SetTextSize(26);
	}
	{
		auto& b = AddWidget(std::make_unique<Button>(Renderer(), L"选择", 120, 40));
		b.anchor = Anchor::Center;
		b.x = -540;
		b.y = -130;
		b.SetTextSize(24);
		b.SetClickEvent([&](auto&)->Utils::Task
			{
				auto path = co_await OpenFile({ {L"存档文件(*.gil)",L"*.gil"},{L"所有文件(*.*)",L"*.*"} });
				Flush();
				if (path.empty()) co_return;
				project_path->SetText(Renderer(), path);
				project_path->SetTextSize(22);
				pp = path;
			});
	}
	{
		auto& b = AddWidget(std::make_unique<Button>(Renderer(), L"选择", 120, 40));
		b.anchor = Anchor::Center;
		b.x = -540;
		b.y = -30;
		b.SetTextSize(24);
		b.SetClickEvent([&](auto&)->Utils::Task
			{
				auto path = co_await OpenFile({});
				Flush();
				if (path.empty()) co_return;
				script_dir->SetText(Renderer(), path);
				script_dir->SetTextSize(22);
				sd = path;
			});
	}
	{
		auto& r = AddWidget(std::make_unique<UI::Rectangle>(Renderer(), 640, 40));
		r.anchor = Anchor::Center;
		r.x = -256;
		r.y = -130;
		auto& t = AddWidget(std::make_unique<TextBox>(Renderer(), L"<未选择文件>", 4600));
		project_path = &t;
		t.anchor = Anchor::Center;
		t.x = -250;
		t.y = -125;
		t.SetTextSize(22);
	}
	{
		auto& r = AddWidget(std::make_unique<UI::Rectangle>(Renderer(), 640, 40));
		r.anchor = Anchor::Center;
		r.x = -256;
		r.y = -30;
		auto& t = AddWidget(std::make_unique<TextBox>(Renderer(), L"<未选择目录>", 4600));
		script_dir = &t;
		t.anchor = Anchor::Center;
		t.x = -250;
		t.y = -25;
		t.SetTextSize(22);
	}
	{
		auto& b = AddWidget(std::make_unique<Button>(Renderer(), L"编译并写入", 160));
		b.anchor = Anchor::Center;
		b.x = -200;
		b.y = 100;
		b.SetOriginCenter();
		b.SetTextSize(26);
		b.SetClickEvent([&](auto&)->Utils::Task
			{
				if (auto flag = false; !lock.compare_exchange_strong(flag, true)) co_return;
				Utils::Future<void> future;
				try
				{
					if (!std::filesystem::exists(pp)) throw std::runtime_error("Project file not exists");
					if (!(std::filesystem::exists(sd) && std::filesystem::is_directory(sd))) throw std::runtime_error("Script directory not exists");
					Tools::Compiler compiler(Ugc::NodeGraph::LoadProject(pp));
					int count = 0;
					for (auto& entry : std::filesystem::directory_iterator(sd))
					{
						if (!std::filesystem::is_regular_file(entry.status())) continue;
						if (!entry.path().string().ends_with(".gis")) continue;
						compiler.AddModule((char*)entry.path().stem().u8string().data(), LoadScript(entry.path()));
						++count;
					}
					compiler.Compile();
					compiler.Write();
					compiler.Release()->Save(pp);
					co_await Dialog(L"提示", std::format(L"写入完成。共编译 {} 个文件。", count), MB_ICONINFORMATION);
					lock = false;
					co_return;
				}
				catch (std::exception& e)
				{
					future = Dialog(L"错误", Utils::ToUtf16(e.what()).data(), MB_ICONERROR);
				}
				catch (...)
				{
					future = Dialog(L"错误", L"未知错误", MB_ICONERROR);
				}
				co_await future;
				lock = false;
			});
	}
	{
		auto& b = AddWidget(std::make_unique<Button>(Renderer(), L"打开编辑器", 160));
		b.anchor = Anchor::Center;
		b.x = 200;
		b.y = 100;
		b.SetOriginCenter();
		b.SetTextSize(26);
		b.SetClickEvent([&](auto&)->Utils::Task { co_await Dialog(L"开发中", L"请等待后续更新", MB_ICONINFORMATION); });
	}
	{
		auto [data, size] = Utils::GetResource(IDB_PNG1, L"png");
		auto& b = AddWidget(std::make_unique<ImageButton>(Renderer(), Tool::Image::LoadBitmapFromMemory(Renderer().D2DCtx(), data, size, 60, 60)));
		b.anchor = Anchor::RightBottom;
		b.x = -60;
		b.y = -60;
		b.SetClickEvent([&](auto&) { ShellExecuteW(nullptr, L"open", L"https://www.bilibili.com/video/BV1kCiBBPEWu", nullptr, nullptr, SW_SHOWNORMAL); });
	}
	{
		auto [data, size] = Utils::GetResource(IDB_PNG2, L"png");
		auto& b = AddWidget(std::make_unique<ImageButton>(Renderer(), Tool::Image::LoadBitmapFromMemory(Renderer().D2DCtx(), data, size, 60, 60)));
		b.anchor = Anchor::RightBottom;
		b.x = -120;
		b.y = -60;
		b.SetClickEvent([&](auto&) { ShellExecuteW(nullptr, L"open", L"https://github.com/hackermdch/GIScriptEditor", nullptr, nullptr, SW_SHOWNORMAL); });
	}
	{
		auto& b = AddWidget(std::make_unique<Button>(Renderer(), L"☰", 40, 40));
		b.anchor = Anchor::LeftBottom;
		b.y = -40;
		b.SetStyle(Renderer().Style().Color(0.8, 0.8, 0.8, 0.3).Build());
		b.SetStyle(Renderer().Style().Color(1, 0.8, 0.6, 0.5).Build(), 2);
		b.SetStyle(Renderer().Style().Color(0.3, 0.3, 0.3, 1).Line(3).Build(), 3);
		b.SetStyle(Renderer().Style().Color(0.8, 0.8, 0.8, 0.8).Build(), 4);
		b.SetTextSize(24);
		auto& pm = AddWidget(std::make_unique<PopMenu>(Renderer(), std::vector<std::wstring>{L"复合节点引用查找", L"更多……"}, 160, 30));
		pm.anchor = Anchor::LeftBottom;
		pm.y = -60;
		pm.SetTextSize(16);
		pm.SetTitle(Renderer(), L"额外工具：", 25);
		pm.SetClickEvent([&](auto&, int index)->Utils::Task
			{
				switch (index)
				{
				case 0:
					sub_window = std::make_unique<FindReferenceWindow>();
					sub_window->Join(this);
					break;
				case 1:
					co_await Dialog(L"想不出来", L"这里还没有东西哦", MB_ICONINFORMATION);
					break;
				}
			});
		b.SetClickEvent([&](auto&)
			{
				SetPop(pm);
				Flush();
			});
	}
}
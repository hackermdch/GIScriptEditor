#include <Windows.h>
#include <GINodeGraph.h>
#include "resource.h"

import std;
import window;
import util;
import compiler;
import widgets;
import image;

using namespace Editor;
using namespace UI;

auto LoadScript(const std::filesystem::path& path)
{
	std::ifstream in(path);
	std::string code((std::istreambuf_iterator(in)), std::istreambuf_iterator<char>());
	return code;
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
	TextBox* project_path;
	TextBox* script_dir;
	std::wstring pp, sd;
	Base::Window window("GIScriptEditor");
	{
		auto& t = window.AddWidget(std::make_unique<TextBox>(window.Renderer(), L"当前版本：1.0.3", 160));
		t.anchor = Anchor::Top;
		t.y = 40;
		t.SetOriginCenter();
		t.SetTextSize(18);
		t.SetStyle(window.Renderer().Style().Color(0.4, 1, 0.4, 1).Build());
		t.SetStyle(window.Renderer().Style().Color(1, 1, 0, 1).Dilate().Build(), 1);
	}
	{
		auto& t = window.AddWidget(std::make_unique<TextBox>(window.Renderer(), L"存档路径：", 160));
		t.anchor = Anchor::Center;
		t.x = -300;
		t.y = -100;
		t.SetOriginCenter();
		t.SetTextSize(26);
	}
	{
		auto& t = window.AddWidget(std::make_unique<TextBox>(window.Renderer(), L"脚本目录：", 160));
		t.anchor = Anchor::Center;
		t.x = -300;
		t.SetOriginCenter();
		t.SetTextSize(26);
	}
	{
		auto& b = window.AddWidget(std::make_unique<Button>(window.Renderer(), L"选择", 120, 40));
		b.anchor = Anchor::Center;
		b.x = -540;
		b.y = -130;
		b.SetTextSize(24);
		b.SetClickEvent([&](auto&)->Utils::Task
			{
				auto path = co_await window.OpenFile({ {L"存档文件(*.gil)",L"*.gil"},{L"所有文件(*.*)",L"*.*"} });
				if (path.empty()) co_return;
				project_path->SetText(window.Renderer(), path);
				project_path->SetTextSize(22);
				pp = path;
			});
	}
	{
		auto& b = window.AddWidget(std::make_unique<Button>(window.Renderer(), L"选择", 120, 40));
		b.anchor = Anchor::Center;
		b.x = -540;
		b.y = -30;
		b.SetTextSize(24);
		b.SetClickEvent([&](auto&)->Utils::Task
			{
				auto path = co_await window.OpenFile({});
				if (path.empty()) co_return;
				script_dir->SetText(window.Renderer(), path);
				script_dir->SetTextSize(22);
				sd = path;
			});
	}
	{
		auto& r = window.AddWidget(std::make_unique<UI::Rectangle>(window.Renderer(), 640, 40));
		r.anchor = Anchor::Center;
		r.x = -256;
		r.y = -130;
		auto& t = window.AddWidget(std::make_unique<TextBox>(window.Renderer(), L"<未选择文件>", 4600));
		project_path = &t;
		t.anchor = Anchor::Center;
		t.x = -250;
		t.y = -125;
		t.SetTextSize(22);
	}
	{
		auto& r = window.AddWidget(std::make_unique<UI::Rectangle>(window.Renderer(), 640, 40));
		r.anchor = Anchor::Center;
		r.x = -256;
		r.y = -30;
		auto& t = window.AddWidget(std::make_unique<TextBox>(window.Renderer(), L"<未选择目录>", 4600));
		script_dir = &t;
		t.anchor = Anchor::Center;
		t.x = -250;
		t.y = -25;
		t.SetTextSize(22);
	}
	{
		auto& b = window.AddWidget(std::make_unique<Button>(window.Renderer(), L"编译并写入", 160));
		b.anchor = Anchor::Center;
		b.x = -200;
		b.y = 100;
		b.SetOriginCenter();
		b.SetTextSize(26);
		b.SetClickEvent([&](auto&)->Utils::Task
			{
				Utils::Future<void> future;
				try
				{
					if (!std::filesystem::exists(pp)) throw std::runtime_error("Project file not exists");
					if (!(std::filesystem::exists(sd) && std::filesystem::is_directory(sd))) throw std::runtime_error("Script directory not exists");
					auto project = Ugc::NodeGraph::LoadProject(pp);
					Tools::Compiler compiler;
					int count = 0;
					for (auto& entry : std::filesystem::directory_iterator(sd))
					{
						if (!std::filesystem::is_regular_file(entry.status())) continue;
						if (!entry.path().string().ends_with(".gis")) continue;
						compiler.Compile((char*)entry.path().stem().u8string().data(), LoadScript(entry.path()));
						++count;
					}
					compiler.Write(project.get());
					project->Save(pp);
					co_await window.Dialog(L"提示", std::format(L"写入完成。共编译 {} 个文件。", count), MB_ICONINFORMATION);
					co_return;
				}
				catch (std::exception& e)
				{
					future = window.Dialog(L"错误", Utils::ToUtf16(e.what()).data(), MB_ICONERROR);
				}
				catch (...)
				{
					future = window.Dialog(L"错误", L"未知错误", MB_ICONERROR);
				}
				co_await future;
			});
	}
	{
		auto& b = window.AddWidget(std::make_unique<Button>(window.Renderer(), L"打开编辑器", 160));
		b.anchor = Anchor::Center;
		b.x = 200;
		b.y = 100;
		b.SetOriginCenter();
		b.SetTextSize(26);
		b.SetClickEvent([&](auto&)->Utils::Task { co_await window.Dialog(L"开发中", L"请等待后续更新", MB_ICONINFORMATION); });
	}
	{
		auto [data, size] = Utils::GetResource(IDB_PNG1, L"png");
		auto& b = window.AddWidget(std::make_unique<ImageButton>(window.Renderer(), Tool::Image::LoadBitmapFromMemory(window.Renderer().D2DCtx(), data, size, 60, 60)));
		b.anchor = Anchor::RightBottom;
		b.x = -60;
		b.y = -60;
		b.SetClickEvent([&](auto&) { ShellExecuteW(nullptr, L"open", L"https://www.bilibili.com/video/BV1kCiBBPEWu", nullptr, nullptr, SW_SHOWNORMAL); });
	}
	{
		auto [data, size] = Utils::GetResource(IDB_PNG2, L"png");
		auto& b = window.AddWidget(std::make_unique<ImageButton>(window.Renderer(), Tool::Image::LoadBitmapFromMemory(window.Renderer().D2DCtx(), data, size, 60, 60)));
		b.anchor = Anchor::RightBottom;
		b.x = -120;
		b.y = -60;
		b.SetClickEvent([&](auto&) { ShellExecuteW(nullptr, L"open", L"https://github.com/hackermdch/GIScriptEditor", nullptr, nullptr, SW_SHOWNORMAL); });
	}
	return window.Run();
}

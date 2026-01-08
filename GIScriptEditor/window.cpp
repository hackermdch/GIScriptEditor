module;
#include <Windows.h>
module window;

using namespace Editor::Base;

namespace
{
	[[maybe_unused]]
	struct StaticInit
	{
		StaticInit()
		{
			SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
			WNDCLASSW cls{};
			cls.lpszClassName = L"AppWindow";
			cls.lpfnWndProc = DefWindowProcW;
			RegisterClassW(&cls);
		}
	}_;
}

LRESULT Window::ProcMsg(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
	case WM_DESTROY:
		if (standalone_window) PostQuitMessage(0);
		else if (joined)
		{
			EnableWindow(joined->handle, true);
			SetForegroundWindow(joined->handle);
			SetFocus(joined->handle);
		}
		handle = nullptr;
		return 0;
	case WM_PAINT:
		renderer.BeginRender();
		for (auto& w : widgets) if (w->visible) w->Render(renderer);
		renderer.EndRender();
		return 0;
	case WM_SIZE:
		renderer.Resize(LOWORD(lparam), HIWORD(lparam));
		break;
	case WM_SETCURSOR:
		if (LOWORD(lparam) == HTCLIENT)
		{
			SetCursor(LoadCursor(nullptr, IDC_ARROW));
			return true;
		}
		break;
	case WM_MOUSEMOVE:
	{
		auto x = LOWORD(lparam), y = HIWORD(lparam);
		auto old = hovered;
		auto hit = true;
		for (auto& w : widgets | std::views::reverse)
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
		}
		if (hit) hovered = nullptr;
		break;
	}
	case WM_LBUTTONDOWN:
		if (pop_widget && hovered != pop_widget)
		{
			pop_widget->visible = false;
			pop_widget = nullptr;
		}
		if (hovered)
		{
			auto [px, py] = hovered->TransformLocal(LOWORD(lparam), HIWORD(lparam));
			hovered->OnMouseClick(px, py);
		}
		break;
	case WM_MOUSEWHEEL:
		if (pop_widget && hovered != pop_widget)
		{
			pop_widget->visible = false;
			pop_widget = nullptr;
		}
		if (hovered)
		{
			POINT pt{ LOWORD(lparam), HIWORD(lparam) };
			ScreenToClient(hwnd, &pt);
			auto [px, py] = hovered->TransformLocal(pt.x, pt.y);
			hovered->OnMouseWheel(px, py, GET_WHEEL_DELTA_WPARAM(wparam) / (float)WHEEL_DELTA);
		}
		break;
	default: break;
	}
	return DefWindowProcW(hwnd, msg, wparam, lparam);
}

Window::Window(const std::wstring& title) :
	RelativeLayout([&r = renderer] { return r.Width(); }, [&r = renderer] { return r.Height(); }),
	handle(CreateWindowExW(WS_EX_NOREDIRECTIONBITMAP, L"AppWindow", title.data(), WS_OVERLAPPEDWINDOW, 100, 100, 1280, 720, nullptr, nullptr, nullptr, nullptr)),
	message([this](HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) { return ProcMsg(hwnd, msg, wparam, lparam); })
{
	SetWindowLongPtrW(handle, GWLP_WNDPROC, (LONG_PTR)message.Get());
	RECT rc;
	GetClientRect(handle, &rc);
	renderer.Resize(rc.right - rc.left, rc.bottom - rc.top);
	renderer.Bind(handle);
}

Window::~Window()
{
	if (handle) DestroyWindow(handle);
}

int Window::Run()
{
	standalone_window = true;
	MSG msg;
	ProcMsg(handle, WM_PAINT, 0, 0);
	ShowWindow(handle, SW_SHOW);
	while (GetMessageW(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
	return 0;
}

void Window::Join(Window* parent)
{
	if (parent)
	{
		EnableWindow(parent->handle, false);
		joined = parent;
	}
	ProcMsg(handle, WM_PAINT, 0, 0);
	ShowWindow(handle, SW_SHOW);
}

Utils::Future<void> Window::Dialog(const std::wstring& title, const std::wstring& msg, UINT type) const
{
	return Utils::Future(std::async(std::launch::deferred, [=] { MessageBoxW(handle, msg.data(), title.data(), type); }));
}

Utils::Future<std::wstring> Window::OpenFile(const std::vector<std::pair<std::wstring, std::wstring>>& filters) const
{
	return Utils::Future(std::async(std::launch::deferred, [this, fs = filters]
		{
			CoInitializeEx(nullptr, COINIT_MULTITHREADED);
			auto ret = Utils::FileDialogHelper::OpenFile(handle, fs);
			CoUninitialize();
			return ret;
		}));
}

Utils::Future<std::wstring> Window::SaveFile(const std::wstring& default_ext, const std::vector<std::pair<std::wstring, std::wstring>>& filters) const
{
	return Utils::Future(std::async(std::launch::deferred, [this, de = default_ext, fs = filters]
		{
			CoInitializeEx(nullptr, COINIT_MULTITHREADED);
			auto ret = Utils::FileDialogHelper::SaveFile(handle, de, fs);
			CoUninitialize();
			return ret;
		}));
}

void Window::Flush() const
{
	POINT pt;
	RECT rect;
	GetCursorPos(&pt);
	GetWindowRect(handle, &rect);
	if (PtInRect(&rect, pt))
	{
		ScreenToClient(handle, &pt);
		PostMessageW(handle, WM_MOUSEMOVE, 0, MAKELPARAM(pt.x, pt.y));
	}
}

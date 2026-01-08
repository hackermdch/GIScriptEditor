export module app.window;

import std;
import window;
import widgets;

export namespace Editor::App
{
	class MainWindow : public Base::Window
	{
		UI::TextBox* project_path;
		UI::TextBox* script_dir;
		std::wstring pp, sd;
		std::atomic<bool> lock;
		std::unique_ptr<Window> sub_window;
	public:
		MainWindow();
	};

	class FindReferenceWindow : public Base::Window
	{
		UI::ListView* list;
	public:
		FindReferenceWindow();
	};
}

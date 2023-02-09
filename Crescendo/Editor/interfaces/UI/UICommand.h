#pragma once

#include "core/core.h"

#include "ImGui/imgui.h"

namespace Crescendo::Editor::Slick::Commands
{
	class UICommand
	{
	public:
		virtual void Execute() = 0;
	};

	/*
	BeginMenuBar
	BeginMenu(name)
	MenuItem(first, second)
	EndMenu
	EndMenuBar
	TextColored(r, g, b, a, text)
	BeginChild(id)
	Text(text)
	EndChild
	End*/

	class Begin : public UICommand
	{
	private:
		const char* name;
		bool open;
		unsigned int flags;
	public:
		Begin(const char* n, unsigned int f)
		{
			name = n;
			open = true;
			flags = f;
		}
		void Execute()
		{
			ImGui::Begin(name, &open, ImGuiWindowFlags_MenuBar);
		}
	};

	class BeginMenuBar : public UICommand
	{
	public:
		void Execute()
		{
			ImGui::BeginMenu();
		}
	};
}
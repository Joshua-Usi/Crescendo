#pragma once

#include "interfaces/Window.h"

#include "GLFW/glfw3.h"

namespace Crescendo::Engine
{
	class WindowsWindow : public Window
	{
	public:
		WindowsWindow(const WindowProps& props);
		virtual ~WindowsWindow();

		virtual void OnUpdate() override;

		virtual gt::Int32 GetWidth() const override;
		virtual gt::Int32 GetHeight() const override;
		virtual gt::Int32 GetRefreshRate() const override;

		virtual void SetVSync(bool isEnabled) override;

		virtual bool IsVSynced() const override;
		virtual bool IsOpen() const override;

		virtual void* GetNative() const override;
	private:
		virtual void Init(const WindowProps& props);
		virtual void Shutdown();

		GLFWwindow* window;

		struct WindowData
		{
			std::string title = "";
			WindowsWindow* windowPointer = NULL;
			gt::Int32 width = 0, height = 0;
			bool vSync = false;
			bool isOpen = false;
		} data;
	};
}
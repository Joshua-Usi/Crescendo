#pragma once

#include "interfaces/Window.h"
#include "interfaces/GraphicsContext.h"
#include "interfaces/ShaderProgram.h"
#include "interfaces/Buffers.h"
#include "interfaces/VertexArray.h"

#include "GLFW/glfw3.h"

#include <memory>

namespace Crescendo::Engine
{
	class WindowsWindow : public Window
	{
	private:
		virtual void Init(const WindowProps& props);
		virtual void Shutdown();

		GLFWwindow* window;
		std::unique_ptr<Rendering::GraphicsContext> context;

		struct WindowData
		{
			Rendering::RendererAPI::API graphicsAPI;
			WindowsWindow* windowPointer = NULL;
			const char* title = "";
			int32_t width = 0, height = 0;
			bool vSync = false;
			bool isCursorLocked = false;
			bool isOpen = false;
		} data;
	public:
		WindowsWindow(const WindowProps& props);
		virtual ~WindowsWindow();

		virtual void OnUpdate() override;
		virtual void OnLateUpdate() override;

		virtual int32_t GetWidth() const override;
		virtual int32_t GetHeight() const override;
		virtual int32_t GetRefreshRate() const override;

		virtual void SetVSync(bool isEnabled) override;
		virtual void SetCursorLock(bool isEnabled) override;

		virtual bool IsVSynced() const override;
		virtual bool IsCursorLocked() const override;
		virtual bool IsOpen() const override;

		virtual void* GetNative() const override;
	};
}
#pragma once

#include "Engine/interfaces/Window.hpp"

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

namespace Crescendo::Engine
{
	class WindowsWindow : public Window
	{
	private:
		GLFWwindow* window;
	public:
		WindowsWindow(const WindowProperties& props);
		virtual ~WindowsWindow() override;

		WindowsWindow(const WindowsWindow&) = delete;
		WindowsWindow& operator=(const WindowsWindow&) = delete;

		virtual void Close() override final;
		virtual void OnUpdate() override final;
		virtual void OnLateUpdate() override final;

		virtual int32_t GetWidth() const override final;
		virtual int32_t GetHeight() const override final;
		virtual void* GetNative() const override final;
		virtual int32_t GetRefreshRate() const override final;
		virtual std::string GetName() const override final;

		virtual void SetVSync(bool isEnabled) override final;
		virtual void SetCursorLock(bool isEnabled) override final;
		virtual void SetName(const std::string& name) override final;

		virtual bool IsVSynced() const override final;
		virtual bool IsCursorLocked() const override final;
		virtual bool IsOpen() const override final;
	};
}
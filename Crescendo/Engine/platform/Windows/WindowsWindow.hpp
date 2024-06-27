#pragma once

#include "Engine/interfaces/Window.hpp"

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

CS_NAMESPACE_BEGIN
{
	class WindowsWindow : public Window
	{
	private:
		static bool IsGLFWInitialised;
		static uint32_t windowsOpen;
		GLFWwindow* window;
	public:
		WindowsWindow(const Specification& spec);
		virtual ~WindowsWindow() override final;
		WindowsWindow(const WindowsWindow&) = delete;
		WindowsWindow& operator=(const WindowsWindow&) = delete;
		WindowsWindow(WindowsWindow&&) = default;
		WindowsWindow& operator=(WindowsWindow&&) = default;
		virtual void Close() override final;
		virtual void OnUpdate() override final {};
		virtual uint32_t GetWidth() const override final;
		virtual uint32_t GetHeight() const override final;
		virtual float GetAspectRatio() const override final;
		virtual void* GetNative() const override final;
		virtual int32_t GetRefreshRate() const override final;
		virtual std::string GetName() const override final;
		virtual void SetVSync(bool isEnabled) override final;
		virtual void SetCursorLock(bool isEnabled) override final;
		virtual void SetName(const std::string& name) override final;
		virtual void SetFullScreen(bool isFullScreen) override final;
		virtual void SetSize(uint32_t width, uint32_t height) override final;
		virtual bool IsVSynced() const override final;
		virtual bool IsCursorLocked() const override final;
		virtual bool IsOpen() const override final;
		virtual bool IsFullScreen() const override final;
	};
}
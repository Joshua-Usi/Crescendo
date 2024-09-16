#pragma once

#include "Interfaces/Input.hpp"
#include "Interfaces/Window.hpp"
#include "GLFW/glfw3.h"

CS_NAMESPACE_BEGIN
{
	class WindowsInput : public Input
	{
	private:
		GLFWwindow* window;
		bool isKeyDown[static_cast<size_t>(Key::KEY_COUNT)];
		bool isMouseButtonDown[static_cast<size_t>(MouseButton::BUTTON_COUNT)];
	public:
		WindowsInput(const Specification& spec);
		virtual ~WindowsInput() override final = default;
		WindowsInput(const WindowsInput&) = delete;
		WindowsInput& operator=(const WindowsInput&) = delete;
		WindowsInput(WindowsInput&&) = default;
		WindowsInput& operator=(WindowsInput&&) = default;
		virtual bool GetKeyDown(Key keyCode) override final;
		virtual bool GetKeyPressed(Key keyCode) override final;
		virtual double GetMouseX() override final;
		virtual double GetMouseY() override final;
		virtual bool GetMouseDown(MouseButton button) override final;
		virtual bool GetMousePressed(MouseButton button) override final;
		virtual void PollEvents() override final;
	};
}
#include "WindowsInput.hpp"
#include "Platform/Desktop/InputCodeMaps.hpp"

CS_NAMESPACE_BEGIN
{
	// Overload for Input to create WindowsInput
	std::unique_ptr<Input> Input::Create(const Specification& spec) { return std::make_unique<WindowsInput>(spec); }
	WindowsInput::WindowsInput(const Specification& spec) : window(static_cast<GLFWwindow*>(spec.window->GetNative()))
	{
		// Surely there's a better way to do this?
		for (size_t i = 0; i < static_cast<size_t>(Key::KEY_COUNT); i++)
		{
			this->isKeyDown[i] = false;
		}
		for (size_t i = 0; i < static_cast<size_t>(MouseButton::BUTTON_COUNT); i++)
		{
			this->isMouseButtonDown[i] = false;
		}
	}
	bool WindowsInput::GetKeyDown(Key keyCode)
	{
		bool keyPressed = this->GetKeyPressed(keyCode);
		if (keyPressed && !isKeyDown[static_cast<size_t>(keyCode)])
		{
			this->isKeyDown[static_cast<size_t>(keyCode)] = true;
			return true;
		}
		else if (!keyPressed)
		{
			this->isKeyDown[static_cast<size_t>(keyCode)] = false;
		}
		return false;
	}
	bool WindowsInput::GetKeyPressed(Key keyCode)
	{
		int state = glfwGetKey(this->window, KeyToGLFWMapping[static_cast<uint32_t>(keyCode)]);
		return state == GLFW_PRESS;
	}

	double WindowsInput::GetMouseX()
	{
		double position = 0;
		glfwGetCursorPos(this->window, &position, NULL);
		return position;
	}
	double WindowsInput::GetMouseY()
	{
		double position = 0;
		glfwGetCursorPos(this->window, NULL, &position);
		return position;
	}

	bool WindowsInput::GetMouseDown(MouseButton button)
	{
		bool mousePressed = this->GetMousePressed(button);
		if (mousePressed && !isMouseButtonDown[static_cast<size_t>(button)])
		{
			this->isMouseButtonDown[static_cast<size_t>(button)] = true;
			return true;
		}
		else if (!mousePressed)
		{
			this->isMouseButtonDown[static_cast<size_t>(button)] = false;
		}
		return false;
	}
	bool WindowsInput::GetMousePressed(MouseButton button)
	{
		int state = glfwGetMouseButton(this->window, MouseButtonToGLFWMapping[static_cast<uint32_t>(button)]);
		return state == GLFW_PRESS;
	}
	void WindowsInput::PollEvents()
	{
		glfwPollEvents();
	}
}
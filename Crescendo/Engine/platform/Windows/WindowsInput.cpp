#include "WindowsInput.hpp"

#include "Engine/platform/Desktop/InputCodeMaps.hpp"
#include "Engine/Application/Application.hpp"

#include "GLFW/glfw3.h"

CS_NAMESPACE_BEGIN
{
	Input* Input::self = new WindowsInput();

	bool WindowsInput::KeyDownImpl(Key keyCode)
	{
		bool keyPressed = Input::GetKeyPressed(keyCode);
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
	bool WindowsInput::KeyPressedImpl(Key keyCode)
	{
		GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get()->GetWindow()->GetNative());
		int state = glfwGetKey(window, KeyToGLFWMapping[static_cast<uint32_t>(keyCode)]);
		return state == GLFW_PRESS;
	}

	double WindowsInput::MousePositionXImpl()
	{
		GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get()->GetWindow()->GetNative());
		double position = 0;
		glfwGetCursorPos(window, &position, NULL);
		return position;
	}
	double WindowsInput::MousePositionYImpl()
	{
		GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get()->GetWindow()->GetNative());
		double position = 0;
		glfwGetCursorPos(window, NULL, &position);
		return position;
	}

	bool WindowsInput::MouseButtonDownImpl(MouseButton button)
	{
		bool mousePressed = Input::GetMouseButtonPressed(button);
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
	bool WindowsInput::MouseButtonPressedImpl(MouseButton button)
	{
		GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get()->GetWindow()->GetNative());
		int state = glfwGetMouseButton(window, MouseButtonToGLFWMapping[static_cast<uint32_t>(button)]);
		return state == GLFW_PRESS;
	}
	void WindowsInput::PollEventsImpl()
	{
		glfwPollEvents();
	}
}
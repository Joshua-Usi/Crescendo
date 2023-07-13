#include "WindowsInput.hpp"

#include "GLFW/glfw3.h"
#include "Engine/Application/Application.hpp"

namespace Crescendo::Engine
{
	Input* Input::self = new WindowsInput();

	bool WindowsInput::KeyDownImpl(Key keyCode) const
	{
		GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get()->GetWindow()->GetNative());
		int state = glfwGetKey(window, static_cast<int>(keyCode));
		return state == GLFW_PRESS;
	}
	// TODO CRESCENDO pressed implementation
	bool WindowsInput::KeyPressedImpl(Key keyCode) const
	{
		GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get()->GetWindow()->GetNative());
		int state = glfwGetKey(window, static_cast<int>(keyCode));
		return state == GLFW_PRESS;
	}

	double WindowsInput::MousePositionXImpl() const
	{
		GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get()->GetWindow()->GetNative());
		double position = 0;
		glfwGetCursorPos(window, &position, NULL);
		return position;
	}
	double WindowsInput::MousePositionYImpl() const
	{
		GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get()->GetWindow()->GetNative());
		double position = 0;
		glfwGetCursorPos(window, NULL, &position);
		return position;
	}

	bool WindowsInput::MouseButtonDownImpl(MouseButton button) const
	{
		GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get()->GetWindow()->GetNative());
		int state = glfwGetMouseButton(window, static_cast<int>(button));
		return state == GLFW_PRESS;
	}
	// TODO CRESCENDO pressed implementation
	bool WindowsInput::MouseButtonPressedImpl(MouseButton button) const
	{
		GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get()->GetWindow()->GetNative());
		int state = glfwGetMouseButton(window, static_cast<int>(button));
		return state == GLFW_PRESS;
	}
}
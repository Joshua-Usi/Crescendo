#include "WindowsInput.h"

#include "GLFW/glfw3.h"
#include "application/Application.h"

namespace Crescendo::Engine
{
	Input* Input::self = new WindowsInput();

	bool WindowsInput::KeyDownImpl(Key keyCode)
	{
		GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get()->GetWindow()->GetNative());
		int state = glfwGetKey(window, static_cast<int>(keyCode));
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}
	// TODO CRESCENDO pressed implementation
	bool WindowsInput::KeyPressedImpl(Key keyCode)
	{
		GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get()->GetWindow()->GetNative());
		int state = glfwGetKey(window, static_cast<int>(keyCode));
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
		GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get()->GetWindow()->GetNative());
		int state = glfwGetMouseButton(window, static_cast<int>(button));
		return state == GLFW_PRESS;
	}
	// TODO CRESCENDO pressed implementation
	bool WindowsInput::MouseButtonPressedImpl(MouseButton button)
	{
		GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get()->GetWindow()->GetNative());
		int state = glfwGetMouseButton(window, static_cast<int>(button));
		return state == GLFW_PRESS;
	}
}
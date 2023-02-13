#include "core/core.h"

#include "OpenGLGraphicsContext.h"

#include "glfw/glfw3.h"
#include "glad/glad.h"

namespace Crescendo::Rendering
{
	OpenGLGraphicsContext::OpenGLGraphicsContext(GLFWwindow* windHandle)
	{
		this->windowHandle = windHandle;

		CS_ASSERT(windowHandle, "Invalid Window Handle: windowHandle is null!");
	}
	void OpenGLGraphicsContext::Init()
	{
		glfwMakeContextCurrent(this->windowHandle);
		// why does the glad loader check look like this????
		CS_ASSERT(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress), "GLAD Initialisation Failed!");
	}
	void OpenGLGraphicsContext::SwapBuffers()
	{
		glfwSwapBuffers(this->windowHandle);
	}

}
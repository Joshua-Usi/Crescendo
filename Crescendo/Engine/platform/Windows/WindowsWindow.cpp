#include "WindowsWindow.h"

#include "console/console.h"
#include "glad/glad.h"

namespace Crescendo::Engine
{
	static bool IsGLFWInitialised = false;
	Window* Window::Create(const WindowProps& props)
	{
		return new WindowsWindow(props);
	}

	WindowsWindow::WindowsWindow(const WindowProps& props)
	{
		Init(props);
	}

	WindowsWindow::~WindowsWindow()
	{
		Shutdown();
	}

	void WindowsWindow::Init(const WindowProps& props)
	{
		this->data.title = props.title;
		this->data.width = props.width;
		this->data.height = props.height;
		this->data.isOpen = true;
		this->data.windowPointer = this;

		Console::EngineInfo("Creating window \"{}\" of size {}x{}", props.title, props.width, props.height);

		//this->GraphicsContext = new OpenGLGraphicsContext();

		// Initialise glfw if it hasn't been yet
		if (!IsGLFWInitialised)
		{
			CS_ASSERT(glfwInit(), "GLFW Initialisation Failed!");
			IsGLFWInitialised = true;
		}

		// first thing after initialisation
		glfwSetErrorCallback([](int error_code, const char* description)
		{
			Engine::Console::EngineError("GLFW Error ({}): {}", error_code, description);
		});

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		this->window = glfwCreateWindow(this->data.width, this->data.height, this->data.title.c_str(), NULL, NULL);

		//this->GraphicsContext->Init();

		glfwMakeContextCurrent(this->window);

		// why does the glad loader check look like this????
		CS_ASSERT(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress), "GLAD Initialisation Failed!");

		glfwSetWindowUserPointer(this->window, &this->data);

		glfwSetWindowCloseCallback(this->window, [](GLFWwindow* window)
		{
			WindowData windowPointer = CastVoid(WindowData, glfwGetWindowUserPointer(window));
			windowPointer.windowPointer->Shutdown();
		});

		this->SetVSync(true);
	}

	void WindowsWindow::Shutdown()
	{
		this->data.isOpen = false;
		glfwDestroyWindow(this->window);
	}

	void WindowsWindow::OnUpdate()
	{
		if (this->data.isOpen)
		{
			glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);
			glfwPollEvents();
			glfwSwapBuffers(this->window);
		}
	}

	gt::Int32 WindowsWindow::GetWidth() const
	{
		return this->data.width;
	}

	gt::Int32 WindowsWindow::GetHeight() const
	{
		return this->data.height;
	}

	gt::Int32 WindowsWindow::GetRefreshRate() const
	{
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		return glfwGetVideoMode(monitor)->refreshRate;
	}

	void WindowsWindow::SetVSync(bool isEnabled)
	{
		if (isEnabled)
		{
			glfwSwapInterval(1);
		}
		else
		{
			glfwSwapInterval(0);
		}
		this->data.vSync = isEnabled;
	}

	bool WindowsWindow::IsVSynced() const
	{
		return this->data.vSync;
	}

	bool WindowsWindow::IsOpen() const
	{
		return this->data.isOpen;
	}

	void* WindowsWindow::GetNative() const
	{
		return this->window;
	}
}
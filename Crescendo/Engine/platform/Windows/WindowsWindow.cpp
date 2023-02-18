#include "WindowsWindow.h"

#include "filesystem/filesystem.h"
#include "filesystem/synchronous/syncFiles.h"
#include "console/console.h"
#include "glad/glad.h"

#include "Renderer.h"

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

		this->window = glfwCreateWindow(this->data.width, this->data.height, this->data.title, NULL, NULL);

		glfwSetWindowUserPointer(this->window, &this->data);

		glfwSetWindowCloseCallback(this->window, [](GLFWwindow* window)
		{
			WindowData windowPointer = CastVoid(WindowData, glfwGetWindowUserPointer(window));
			windowPointer.windowPointer->Shutdown();
		});

		// Set API
		Rendering::Renderer::SetAPI(Rendering::GraphicsAPI::OpenGL);

		this->context.reset(Rendering::GraphicsContext::Create(this->window));
		this->context->Init();

		this->SetVSync(false);

		// Vertex Array
		this->vertexArray.reset(Rendering::VertexAttributeObject::Create());
		this->vertexArray->Bind();
		// Vertex Buffer
		this->vertexBuffer.reset(Rendering::BufferObject::Create(Rendering::BufferType::Vertex));
		this->vertexBuffer->Bind();

		std::vector<float> vertices = {
			-0.5f, -0.5f, 0.0f,
			 0.5f, -0.5f, 0.0f,
			 0.0f,  0.5f, 0.0f,
		};

		this->vertexBuffer->SetData(vertices.data(), vertices.size());
		this->vertexArray->SetAttributePointer(0, 3, sizeof(float));

		Rendering::BufferLayout layout = {
			{Rendering::ShaderDataType::Float3, "aPosition"},
		};

		/*this->vertexBuffer->SetLayout(layout);*/

		this->vertexArray->EnableAttribute(0);
		// Color Buffer
		this->colorBuffer.reset(Rendering::BufferObject::Create(Rendering::BufferType::Vertex));
		this->colorBuffer->Bind();

		std::vector<float> colors = {
			1.0f, 0.0f, 0.0f, 1.0f,
			0.0f, 1.0f, 0.0f, 1.0f,
			0.0f, 0.0f, 1.0f, 1.0f,
		};

		this->colorBuffer->SetData(colors.data(), colors.size());

		this->vertexArray->SetAttributePointer(1, 4, sizeof(float));
		this->vertexArray->EnableAttribute(1);

		this->vertexArray->Unbind();

		// Index Buffer
		this->indexBuffer.reset(Rendering::BufferObject::Create(Rendering::BufferType::Index));
		this->indexBuffer->Bind();

		std::vector<unsigned int> indices = { 0, 1, 2 };

		this->indexBuffer->SetData(indices.data(), indices.size());

		this->shaderProgram.reset(Rendering::ShaderProgram::Create());

		std::fstream shader;
		std::string shaderSource;
		// Vertex Shader
		FileSystem::Open(&shader, "../Crescendo/Rendering/shaders/base.vert");
		FileSystem::Read(&shader, &shaderSource);
		this->shaderProgram->AttachShader(Rendering::ShaderType::Vertex, shaderSource.c_str());
		// Fragment Shader
		FileSystem::Open(&shader, "../Crescendo/Rendering/shaders/base.frag");
		FileSystem::Read(&shader, &shaderSource);
		this->shaderProgram->AttachShader(Rendering::ShaderType::Fragment, shaderSource.c_str());
		// Always close files after you are done with them
		FileSystem::Close(&shader);

		// Link and bind the shader program;
		this->shaderProgram->Link();
		this->shaderProgram->Bind();
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
			glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);
			glfwPollEvents();

			this->shaderProgram->Bind();
			this->vertexArray->Bind();
			this->indexBuffer->Bind();
			glDrawElements(GL_TRIANGLES, this->indexBuffer->GetCount(), GL_UNSIGNED_INT, 0);
		}
	}
	void WindowsWindow::OnLateUpdate()
	{
		if (this->data.isOpen)
		{
			this->context->SwapBuffers();
		}
	}
	int32_t WindowsWindow::GetWidth() const
	{
		return this->data.width;
	}
	int32_t WindowsWindow::GetHeight() const
	{
		return this->data.height;
	}
	int32_t WindowsWindow::GetRefreshRate() const
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
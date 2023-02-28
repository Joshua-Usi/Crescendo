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
		this->data.graphicsAPI = props.graphicsAPI;
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
		Rendering::RendererAPI::SetAPI(this->data.graphicsAPI);

		this->context.reset(Rendering::GraphicsContext::Create(this->window));
		this->context->Init();

		this->SetVSync(false);

		// Vertex Array
		this->triangleVertexArray.reset(Rendering::VertexArray::Create());

		// Vertex Buffer
		std::vector<float> triangleVertices = {
			-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
			 0.0f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
		};
		std::shared_ptr<Rendering::VertexBuffer> triangleVertexBuffer;
		triangleVertexBuffer.reset(Rendering::VertexBuffer::Create(triangleVertices.data(), triangleVertices.size()));
		triangleVertexBuffer->SetLayout({
			{Rendering::ShaderDataType::Float3, "aPosition"},
			{Rendering::ShaderDataType::Float4, "aColor"},
		});

		this->triangleVertexArray->AddVertexBuffer(triangleVertexBuffer);
		// Index Buffer
		std::vector<unsigned int> triangleIndices = { 0, 1, 2 };
		std::shared_ptr<Rendering::IndexBuffer> triangleIndexBuffer;
		triangleIndexBuffer.reset(Rendering::IndexBuffer::Create(triangleIndices.data(), triangleIndices.size()));

		this->triangleVertexArray->SetIndexBuffer(triangleIndexBuffer);

		this->squareVertexArray.reset(Rendering::VertexArray::Create());

		std::vector<float> squareVertices = {
			-0.75f, -0.75f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			 0.75f, -0.75f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
			 0.75f,  0.75f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
			-0.75f,  0.75f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
		};

		std::shared_ptr<Rendering::VertexBuffer> squareVertexBuffer;
		squareVertexBuffer.reset(Rendering::VertexBuffer::Create(squareVertices.data(), squareVertices.size()));
		squareVertexBuffer->SetLayout({
			{Rendering::ShaderDataType::Float3, "aPosition"},
			{Rendering::ShaderDataType::Float4, "aColor"},
		});

		this->squareVertexArray->AddVertexBuffer(squareVertexBuffer);

		std::vector<uint32_t> squareIndices = {
			0, 1, 2, 2, 3, 0,
		};
		std::shared_ptr<Rendering::IndexBuffer> squareIndexBuffer;
		squareIndexBuffer.reset(Rendering::IndexBuffer::Create(squareIndices.data(), squareIndices.size()));
		this->squareVertexArray->SetIndexBuffer(squareIndexBuffer);

		std::fstream shader;
		std::string vertexSource;
		std::string fragmentSource;

		// Vertex Shader
		FileSystem::Open(shader, "../Crescendo/Rendering/shaders/base.vert");
		FileSystem::Read(shader, vertexSource);
		// Fragment Shader
		FileSystem::Open(shader, "../Crescendo/Rendering/shaders/base.frag");
		FileSystem::Read(shader, fragmentSource);
		// Always close files after you are done with them
		FileSystem::Close(shader);

		this->shaderProgram.reset(Rendering::ShaderProgram::Create(vertexSource.data(), fragmentSource.data()));
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
			glfwPollEvents();

			this->shaderProgram->Bind();

			Rendering::RenderCommand::Clear();

			Rendering::Renderer::BeginScene();

			Rendering::Renderer::Submit(this->squareVertexArray);
			Rendering::Renderer::Submit(this->triangleVertexArray);

			Rendering::Renderer::EndScene();
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
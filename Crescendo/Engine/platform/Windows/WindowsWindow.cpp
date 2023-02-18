#include "WindowsWindow.h"

#include "filesystem/filesystem.h"
#include "filesystem/synchronous/syncFiles.h"
#include "console/console.h"
#include "glad/glad.h"

#include "Renderer.h"

namespace Crescendo::Engine
{
	static GLenum ShaderDataTypeToOpenGLBaseType(Rendering::ShaderDataType type)
	{
		switch (type) {
		case Rendering::ShaderDataType::Bool: return GL_BOOL;

		case Rendering::ShaderDataType::Int: return GL_INT;
		case Rendering::ShaderDataType::Int2: return GL_INT;
		case Rendering::ShaderDataType::Int3: return GL_INT;
		case Rendering::ShaderDataType::Int4: return GL_INT;

		case Rendering::ShaderDataType::Float: return GL_FLOAT;
		case Rendering::ShaderDataType::Float2: return GL_FLOAT;
		case Rendering::ShaderDataType::Float3: return GL_FLOAT;
		case Rendering::ShaderDataType::Float4: return GL_FLOAT;

		case Rendering::ShaderDataType::Double: return GL_DOUBLE;
		case Rendering::ShaderDataType::Double2: return GL_DOUBLE;
		case Rendering::ShaderDataType::Double3: return GL_DOUBLE;
		case Rendering::ShaderDataType::Double4: return GL_DOUBLE;

		case Rendering::ShaderDataType::Mat3: return GL_FLOAT;
		case Rendering::ShaderDataType::Mat4: return GL_FLOAT;
		}

		CS_ASSERT(false, "Unknown ShaderDataType");
		return 0;
	}
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
		glGenVertexArrays(1, &this->vertexArray);
		glBindVertexArray(this->vertexArray);
		// Vertex Buffer
		std::vector<float> vertices = {
			-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
			 0.0f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
		};
		this->vertexBuffer.reset(Rendering::VertexBuffer::Create(vertices.data(), vertices.size()));
		Rendering::BufferLayout layout = {
			{Rendering::ShaderDataType::Float3, "aPosition"},
			{Rendering::ShaderDataType::Float4, "aColor"},
		};
		this->vertexBuffer->SetLayout(layout);

		uint32_t index = 0;
		for (const auto& element : layout)
		{
			glEnableVertexAttribArray(index);
			glVertexAttribPointer(
				index,
				element.GetComponentCount(),
				ShaderDataTypeToOpenGLBaseType(element.type),
				element.normalized ? GL_TRUE : GL_FALSE,
				layout.GetStride(),
				(const void*)element.offset);
			index++; 
		}
		glBindVertexArray(0);
		// Index Buffer
		std::vector<unsigned int> indices = { 0, 1, 2 };
		this->indexBuffer.reset(Rendering::IndexBuffer::Create(indices.data(), indices.size()));
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
			glBindVertexArray(this->vertexArray);
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
#include "Crescendo.h"
using namespace Crescendo::Engine;
namespace Rendering = Crescendo::Rendering;

class Sandbox : public Application
{
private:
	std::unique_ptr<Rendering::ShaderProgram> shaderProgram;
	
	std::shared_ptr<Rendering::VertexArray> triangleVertexArray;
	std::shared_ptr<Rendering::VertexArray> squareVertexArray;
public:
	void OnStartup()
	{
		Console::BeginFileLog("Sandbox");
		this->triangleVertexArray.reset(Rendering::VertexArray::Create());
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
		FileSystem::Open(shader, "../Crescendo/Rendering/shaders/base.vert");
		FileSystem::Read(shader, vertexSource);
		FileSystem::Open(shader, "../Crescendo/Rendering/shaders/base.frag");
		FileSystem::Read(shader, fragmentSource);
		FileSystem::Close(shader);
		this->shaderProgram.reset(Rendering::ShaderProgram::Create(vertexSource.data(), fragmentSource.data()));
		this->shaderProgram->Bind();
	}
	void OnUpdate()
	{
		this->shaderProgram->Bind();

		Rendering::RenderCommand::Clear();
		Rendering::Renderer::BeginScene();

		Rendering::Renderer::Submit(this->squareVertexArray);
		Rendering::Renderer::Submit(this->triangleVertexArray);

		Rendering::Renderer::EndScene();
	}
	void OnExit()
	{
		Console::EndFileLog();
	}
};

Application* Crescendo::Engine::CreateApplication() {
	return new Sandbox();
}
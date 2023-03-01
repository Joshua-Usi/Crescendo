#include "Crescendo.h"
using namespace Crescendo::Engine;
namespace Rendering = Crescendo::Rendering;

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

class Sandbox : public Application
{
private:
	glm::vec3 camera = glm::vec3(0.0f, 0.0f, 3.0f);

	std::unique_ptr<Rendering::ShaderProgram> shaderProgram;

	std::shared_ptr<Rendering::VertexArray> cubeVertexArray;
public:
	void OnStartup()
	{
		Console::BeginFileLog("Sandbox");
		this->cubeVertexArray.reset(Rendering::VertexArray::Create());
		std::vector<float> cubeVertices = {
			// front
			-0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f,
			 0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f,
			 0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 1.0f,
			-0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f,
			// back 
			-0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f,
			 0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f,
			 0.5f,  0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f,
			-0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f,
		};
		std::shared_ptr<Rendering::VertexBuffer> cubeVertexBuffer;
		cubeVertexBuffer.reset(Rendering::VertexBuffer::Create(cubeVertices.data(), cubeVertices.size()));
		cubeVertexBuffer->SetLayout({
			{Rendering::ShaderDataType::Float3, "aPosition"},
			{Rendering::ShaderDataType::Float4, "aColor"},
		});
		this->cubeVertexArray->AddVertexBuffer(cubeVertexBuffer);
		std::vector<uint32_t> cubeIndices = {
			// front
			0, 1, 2,
			2, 3, 0,
			// right
			1, 5, 6,
			6, 2, 1,
			// back
			7, 6, 5,
			5, 4, 7,
			// left
			4, 0, 3,
			3, 7, 4,
			// bottom
			4, 5, 1,
			1, 0, 4,
			// top
			3, 2, 6,
			6, 7, 3
		};
		std::shared_ptr<Rendering::IndexBuffer> cubeIndexBuffer;
		cubeIndexBuffer.reset(Rendering::IndexBuffer::Create(cubeIndices.data(), cubeIndices.size()));
		this->cubeVertexArray->SetIndexBuffer(cubeIndexBuffer);
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

		/*const float radius = 10.0f;
		float camX = sin(Application::Get()->GetTime()) * radius;
		float camZ = cos(Application::Get()->GetTime()) * radius;
		glm::mat4 view;
		view = glm::lookAt(glm::vec3(camX, 0.0, camZ), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));*/

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, (float)Application::Get()->GetTime(), glm::vec3(0.0f, 0.0f, 1.0f));

		glm::mat4 view = glm::mat4(1.0f);
		// note that we're translating the scene in the reverse direction of where we want to move
		view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));

		glm::mat4 projection;
		projection = glm::perspective(glm::radians(45.0f), (float)Application::GetWindow()->GetWidth() / (float)Application::GetWindow()->GetHeight(), 0.1f, 1000.0f);

		this->shaderProgram->Bind();
		this->shaderProgram->SetMat4("uModel", model);
		this->shaderProgram->SetMat4("uView", view);
		this->shaderProgram->SetMat4("uProjection", projection);

		Rendering::RenderCommand::Clear();
		Rendering::RenderCommand::SetDepthTest(true);
		Rendering::Renderer::BeginScene();

		Rendering::Renderer::Submit(this->cubeVertexArray);

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
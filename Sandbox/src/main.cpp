#include "Crescendo.h"
#include "cameras/Perspective/PerspectiveCamera.h"
#include "random/random.h"

using namespace Crescendo::Engine;
namespace Rendering = Crescendo::Rendering;

class Sandbox : public Application
{
private:
	Rendering::PerspectiveCamera camera = Rendering::PerspectiveCamera(45.0f, 16.0f / 9.0f);

	glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);

	float yaw = 0.0f, pitch = 0.0f, roll = 0.0f;

	float px, py;

	std::vector<glm::vec3> cubePositions;

	std::unique_ptr<Rendering::ShaderProgram> shaderProgram;
	std::shared_ptr<Rendering::VertexArray> cubeVertexArray;
public:
	void OnStartup()
	{
		cubePositions.push_back(glm::vec3(0.0f, 5.0f, 0.0f));
		cubePositions.push_back(glm::vec3(0.0f, -5.0f, 0.0f));
		for (int32_t i = 0; i < 500; i++)
		{
			cubePositions.push_back(glm::vec3(Crescendo::Random::FloatBetween(-5, 5), Crescendo::Random::FloatBetween(0, 0), Crescendo::Random::FloatBetween(-100, 100)));
		}

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
			0, 1, 2, 2, 3, 0, // front
			1, 5, 6, 6, 2, 1, // right
			7, 6, 5, 5, 4, 7, // back
			4, 0, 3, 3, 7, 4, // left
			4, 5, 1, 1, 0, 4, // bottom
			3, 2, 6, 6, 7, 3, // top
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
		Window* appWindow = Application::GetWindow();

		this->shaderProgram->Bind();

		float x = Input::GetMousePositionX(), y = Input::GetMousePositionY();
		float dx = x - this->px, dy = y - this->py;

		px = x, py = y;

		this->yaw -= dx * 0.001f;
		this->pitch -= dy * 0.001f;

		if (pitch > 3.14159f / 2.0f - 0.01f) pitch = 3.14159f / 2.0f - 0.01f;
		if (pitch < -3.14159f / 2.0f + 0.01f) pitch = -3.14159f / 2.0f + 0.01f;

		this->camera.SetRotation(glm::vec3(this->yaw, this->pitch, this->roll));

		glm::vec3 rotation = this->camera.GetRotation();

		if (Input::GetKeyPressed(Key::Space)) this->position.y += 0.1f;
		if (Input::GetKeyPressed(Key::ShiftLeft)) this->position.y -= 0.1f;

		if (Input::GetKeyPressed(Key::W)) this->position.x -= 0.1f * sin(this->yaw), this->position.z -= 0.1f * cos(this->yaw);
		if (Input::GetKeyPressed(Key::S)) this->position.x += 0.1f * sin(this->yaw), this->position.z += 0.1f * cos(this->yaw);

		if (Input::GetKeyPressed(Key::A)) this->position.x -= 0.1f * sin(this->yaw + 3.14159f / 2.0f), this->position.z -= 0.1f * cos(this->yaw + 3.14159f / 2.0f);
		if (Input::GetKeyPressed(Key::D)) this->position.x += 0.1f * sin(this->yaw + 3.14159f / 2.0f), this->position.z += 0.1f * cos(this->yaw + 3.14159f / 2.0f);

		this->camera.SetPosition(this->position);

		glm::mat4 pv = this->camera.GetViewProjectionMatrix();
		this->shaderProgram->SetMat4("uProjectionView", pv);

		Rendering::RenderCommand::Clear();
		Rendering::RenderCommand::SetDepthTest(true);
		Rendering::Renderer::BeginScene();

		for (uint32_t i = 0; i < cubePositions.size(); i++)
		{
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, cubePositions[i]);
			float angle = 20.0f * i;
			model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
			this->shaderProgram->SetMat4("uModel", model);
			Rendering::Renderer::Submit(this->cubeVertexArray);
		}

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
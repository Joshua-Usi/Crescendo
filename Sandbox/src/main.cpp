#include "Crescendo.h"
#include "cameras/Perspective/PerspectiveCamera.h"
#include "Random/Random.h"

using namespace Crescendo::Engine;
namespace Rendering = Crescendo::Rendering;

class Sandbox : public Application
{
private:
	Rendering::PerspectiveCamera camera = Rendering::PerspectiveCamera(glm::radians(45.0f), 16.0f / 9.0f);

	glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);

	float yaw = 0.0f, pitch = 0.0f, roll = 0.0f;

	float px, py;

	std::vector<glm::vec3> cubePositions;

	std::shared_ptr<Rendering::Shader> shader;
	std::shared_ptr<Rendering::VertexArray> cubeVertexArray;
public:
	void OnStartup()
	{
		cubePositions.push_back(glm::vec3(0.0f, 5.0f, 0.0f));
		cubePositions.push_back(glm::vec3(0.0f, -5.0f, 0.0f));
		for (int32_t i = 0; i < 50; i++)
		{
			cubePositions.push_back(glm::vec3(Crescendo::Random::Float(-5, 5), Crescendo::Random::Float(0, 0), Crescendo::Random::Float(-100, 100)));
		}

		Console::BeginFileLog("Sandbox");
		this->cubeVertexArray.reset(Rendering::VertexArray::Create());
		std::vector<float> cubeVertices = {
			// front
			-0.5f, -0.5f,  0.5f,
			 0.5f, -0.5f,  0.5f, 
			 0.5f,  0.5f,  0.5f, 
			-0.5f,  0.5f,  0.5f, 
			// back 
			-0.5f, -0.5f, -0.5f, 
			 0.5f, -0.5f, -0.5f, 
			 0.5f,  0.5f, -0.5f, 
			-0.5f,  0.5f, -0.5f,
		};
		std::shared_ptr<Rendering::VertexBuffer> cubeVertexBuffer;
		cubeVertexBuffer.reset(Rendering::VertexBuffer::Create(cubeVertices.data(), cubeVertices.size()));
		cubeVertexBuffer->SetLayout({
			{Rendering::ShaderDataType::Float3, "aPosition"},
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
		std::string vertexSource;
		std::string fragmentSource;
		FileSystem::Read("../Crescendo/Rendering/shaders/base.vert", vertexSource);
		FileSystem::Read("../Crescendo/Rendering/shaders/base.frag", fragmentSource);
		this->shader.reset(Rendering::Shader::Create(vertexSource.data(), fragmentSource.data()));
	}
	void OnUpdate(double dt)
	{
		Window* appWindow = Application::GetWindow();

		float x = Input::GetMousePositionX(), y = Input::GetMousePositionY();
		float dx = x - this->px, dy = y - this->py;

		px = x, py = y;

		this->yaw -= dx * 0.001f;
		this->pitch -= dy * 0.001f;

		if (pitch > 3.14159f / 2.0f - 0.01f) pitch = 3.14159f / 2.0f - 0.01f;
		if (pitch < -3.14159f / 2.0f + 0.01f) pitch = -3.14159f / 2.0f + 0.01f;

		this->camera.SetRotation(glm::vec3(this->yaw, this->pitch, this->roll));

		glm::vec3 rotation = this->camera.GetRotation();

		glm::vec3 velocity = glm::vec3(0.0f, 0.0f, 0.0f);

		if (Input::GetKeyPressed(Key::W)) velocity.x -= sin(this->yaw) * dt, velocity.z -= cos(this->yaw) * dt;
		if (Input::GetKeyPressed(Key::S)) velocity.x += sin(this->yaw) * dt, velocity.z += cos(this->yaw) * dt;

		if (Input::GetKeyPressed(Key::A)) velocity.x -= sin(this->yaw + 3.14159f / 2.0f) * dt, velocity.z -= cos(this->yaw + 3.14159f / 2.0f) * dt;
		if (Input::GetKeyPressed(Key::D)) velocity.x += sin(this->yaw + 3.14159f / 2.0f) * dt, velocity.z += cos(this->yaw + 3.14159f / 2.0f) * dt;

		if (Input::GetKeyPressed(Key::Space)) velocity.y += dt;
		if (Input::GetKeyPressed(Key::ShiftLeft)) velocity.y -= dt;
		
		if (Input::GetKeyPressed(Key::Escape)) this->Close();

		this->position += velocity * 10.0f;

		this->camera.SetPosition(this->position);

		Rendering::RenderCommand::SetClear(0.5f, 0.5f, 0.5f);
		Rendering::RenderCommand::Clear();
		Rendering::RenderCommand::SetDepthTest(true);

		Rendering::Renderer::BeginScene(this->camera);

		for (uint32_t i = 0; i < cubePositions.size(); i++)
		{
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, cubePositions[i]);
			float angle = 20.0f * i;
			model = glm::rotate(model, glm::radians(angle), { 1.0f, 0.3f, 0.5f });
			float r = ((i + 0) % 3 == 0) ? 1.0f : 0.0f;
			float g = ((i + 1) % 3 == 0) ? 1.0f : 0.0f;
			float b = ((i + 2) % 3 == 0) ? 1.0f : 0.0f;
			this->shader->SetFloat4("uColor", { r, g, b, 1.0f });

			Rendering::Renderer::Submit(this->shader, this->cubeVertexArray, model);
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
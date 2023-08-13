#include "Crescendo.hpp"

using namespace Crescendo::Engine;
typedef Crescendo::Renderer Renderer;
namespace Graphics = Crescendo::Graphics;
namespace Math = Crescendo::Math;
namespace IO = Crescendo::IO;

#include "glm/gtx/common.hpp"
#include "glm/gtc/matrix_transform.hpp"

class Sandbox : public Application
{
private:
	Crescendo::Graphics::Camera camera;
	float pMouseX, pMouseY, sens;

	uint32_t meshCount = 0;

	Renderer renderer;

	int frame = 0;
	double lastTime = 0.0;
public:
	void OnStartup()
	{
		CVar::LoadConfigXML("Config.xml");

		this->GetWindow()->SetCursorLock(true);

		this->sens = CVar::Get<double>("sensitivity");
		this->camera = Graphics::Camera(70.0f, this->GetWindow()->GetAspectRatio(), { 0.1f, 100000.0f });
		this->camera.SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));

		Renderer::BuilderInfo info;

		info.useValidationLayers = true;
		info.preferredDeviceType = Renderer::BuilderInfo::DeviceType::Discrete;
		info.appName = "Sandbox";
		info.engineName = "Crescendo";
		info.window = this->GetWindow()->GetNative();
		info.windowExtent = { this->GetWindow()->GetWidth(), this->GetWindow()->GetHeight() };
		info.preferredPresentMode = Renderer::BuilderInfo::PresentMode::Mailbox;
		info.framesInFlight = 3; // Triple buffering
		info.vertexBufferBlockSize = std::powl(2, 25); // 32MB
		info.descriptorBufferBlockSize = std::powl(2, 18); // 256KB

		this->renderer = Renderer::Create(info);

		// Upload meshes
		IO::Model model;

		CS_TIME(model = IO::LoadOBJ("./assets/sponza.obj"), "Model load");

		this->meshCount = model.meshes.size();
		for (const auto& mesh : model.meshes)
		{
			this->renderer.UploadMesh(mesh.vertices, mesh.normals, mesh.textureUVs, mesh.indices);
		}

		// Upload shaders (creates pipelines and descriptor sets)
		// Shader loading
		std::vector<std::string> shaderList =
		{
			"./shaders/compiled/mesh",
			"./shaders/compiled/mesh-unlit",
		};
		for (size_t i = 0; i < shaderList.size(); i++)
		{
			this->renderer.UploadPipeline(
				Crescendo::Core::BinaryFile(shaderList[i] + ".vert.spv").Open().Read(),
				Crescendo::Core::BinaryFile(shaderList[i] + ".frag.spv").Open().Read()
			);
		}
	}
	void OnUpdate(double dt)
	{
		// Frame counter
		frame++;
		if (this->GetTime() - this->lastTime >= 1.0)
		{
			this->lastTime = this->GetTime();
			this->GetWindow()->SetName("Crescendo | FPS: " + std::to_string(this->frame) + " | Dc: " + std::to_string(this->meshCount));
			this->frame = 0;
		}

		// Camera angles
		double dx = this->pMouseX - Input::GetMouseX(), dy = this->pMouseY - Input::GetMouseY();
		this->pMouseX = Input::GetMouseX(), this->pMouseY = Input::GetMouseY();
		glm::vec3 rotation = camera.GetRotation();
		rotation.x += dx * sens;
		rotation.y = std::clamp(rotation.y + dy * sens, -Math::PI_2 + 0.01f, Math::PI_2 - 0.01f);
		this->camera.SetRotation(rotation);

		// Camera movement
		float velocity = 1.0f;
		glm::vec3 movement(0.0f, 0.0f, 0.0f);
		if (Input::GetKeyPressed(Key::R)) velocity = 10.0f;

		float sinX = std::sin(rotation.x) * velocity, cosX = std::cos(rotation.x) * velocity;

		if (Input::GetKeyPressed(Key::W)) movement.x -= sinX, movement.z -= cosX;
		if (Input::GetKeyPressed(Key::S)) movement.x += sinX, movement.z += cosX;
		if (Input::GetKeyPressed(Key::A)) movement.x -= cosX, movement.z += sinX;
		if (Input::GetKeyPressed(Key::D)) movement.x += cosX, movement.z -= sinX;

		if (Input::GetKeyPressed(Key::Space)) movement.y += 1.0f * velocity;
		if (Input::GetKeyPressed(Key::ShiftLeft)) movement.y -= 1.0f * velocity;
		this->camera.MovePosition(movement);

		// Render prep
		struct VP { glm::mat4 view, projection; } vp { this->camera.GetViewMatrix(), this->camera.GetProjectionMatrix() };
		struct Lighting { glm::vec4 lightColor; float ambient; } lighting { glm::vec4(0.75f, 0.75f, 1.0f, 1.0f), 0.1f };
		glm::mat4 model = glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));

		this->renderer.UpdateDescriptorSetData(0, 0, vp);
		this->renderer.UpdateDescriptorSetData(0, 1, lighting);
		this->renderer.UpdateDescriptorSetData(1, 0, vp);

		// Render commands
		this->renderer.CmdBeginFrame(0.0f, 0.0f, 0.1f, 1.0f);
		this->renderer.CmdBindPipeline(Input::GetKeyDown(Key::One) ? 1 : 0);
		this->renderer.CmdUpdatePushConstant(Renderer::ShaderStage::Vertex, model);
		for (uint32_t i = 0; i < this->meshCount; i++) this->renderer.CmdDraw(i);
		this->renderer.CmdEndFrame();
		this->renderer.CmdPresentFrame();

		if (Input::GetKeyDown(Key::Escape)) this->Exit();
	}
	void OnExit()
	{
		Renderer::Destroy(this->renderer);
	}
};

CrescendoRegisterApp(Sandbox);
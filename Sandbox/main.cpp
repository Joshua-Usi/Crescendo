#include "Crescendo.hpp"

using namespace Crescendo::Engine;
typedef Crescendo::Renderer Renderer;
namespace Graphics = Crescendo::Graphics;
namespace Math = Crescendo::Math;
namespace IO = Crescendo::IO;

#include "glm/gtx/common.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <unordered_map>

class Sandbox : public Application
{
private:
	Crescendo::Graphics::Camera camera = {};
	float pMouseX = 0.0f, pMouseY = 0.0f, sens = 1.0f;

	uint32_t meshCount = 0;

	std::vector<uint32_t> textureIDs;

	Renderer renderer = {};

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
		info.vertexBufferBlockSize = std::powl(2, 24); // use 24 for sponza, 28 for rungholt, 29 for san miguel
		info.descriptorBufferBlockSize = std::powl(2, 18); // 256KB

		this->renderer = Renderer::Create(info);

		// Upload meshes
		IO::Model model;

		CS_TIME(model = IO::LoadOBJ("./assets/sponza.obj"), "Sponza Model load");
		//CS_TIME(model = IO::LoadOBJ("./assets/rungholt.obj"), "Rungholt Model load");
		//CS_TIME(model = IO::LoadOBJ("./assets/san-miguel.obj"), "San Miguel Model load");

		this->meshCount = model.meshes.size();
		uint32_t triangleCount = 0, bufferSpace[4] = { 0, 0, 0, 0 };
		for (const auto& mesh : model.meshes)
		{
			triangleCount += mesh.indices.size() / 3;
			bufferSpace[0] += mesh.vertices.size() * sizeof(float);
			bufferSpace[1] += mesh.normals.size() * sizeof(float);
			bufferSpace[2] += mesh.textureUVs.size() * sizeof(float);
			bufferSpace[3] += mesh.indices.size() * sizeof(uint32_t);
			this->renderer.UploadMesh(mesh.vertices, mesh.normals, mesh.textureUVs, mesh.indices);
		}

		std::cout << "Mesh has " << triangleCount << " triangles" << std::endl;
		std::cout << "Buffer sizes: V:"	<< bufferSpace[0] / 1024 / 1024 << "MB N:"
										<< bufferSpace[1] / 1024 / 1024 << "MB UV:"
										<< bufferSpace[2] / 1024 / 1024 << "MB I:"
										<< bufferSpace[3] / 1024 / 1024 << "MB" << std::endl;

		// Upload shaders (creates pipelines and descriptor sets)
		// Shader loading
		struct Shader { std::string name; Renderer::PipelineVariant variant; };
		std::vector<Shader> shaderList = {
			{"./shaders/compiled/mesh", Renderer::PipelineVariant() },
		};
		for (const auto& shader : shaderList)
		{
			this->renderer.UploadPipeline(
				Crescendo::Core::BinaryFile(shader.name + ".vert.spv").Open().Read(),
				Crescendo::Core::BinaryFile(shader.name + ".frag.spv").Open().Read(),
				{ shader.variant }
			);
		}

		// Upload textures
		std::unordered_map<std::string, uint32_t> seenTextures;
		int i = 0;
		for (const auto& mesh : model.meshes)
		{
			if (mesh.albedo.empty() || seenTextures.find(mesh.albedo) != seenTextures.end())
			{
				this->textureIDs.push_back(seenTextures[mesh.albedo]);
				continue;
			}
			seenTextures[mesh.albedo] = i;
			this->textureIDs.push_back(seenTextures[mesh.albedo]);
			IO::Image image = IO::LoadImage("./assets/" + mesh.albedo);
			this->renderer.UploadTexture(image.pixels, image.width, image.height, image.channels, true);
			i++;
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
		rotation.x += (float)dx * sens;
		rotation.y = std::clamp<float>(rotation.y + dy * sens, -Math::PI_2 + 0.01f, Math::PI_2 - 0.01f);
		this->camera.SetRotation(rotation);

		// Camera movement
		float velocity = 0.1f;
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
		struct Lighting { glm::vec4 lightColor, lightPosition; } lighting{ glm::vec4(0.75f, 0.75f, 1.0f, 1.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) };

		this->renderer.UpdateDescriptorSetData(0, 0, vp);
		this->renderer.UpdateDescriptorSetData(1, 0, lighting);
		this->renderer.UpdateDescriptorSetData(1, 1, glm::vec3(0.5f, 0.0f, 0.0f));

		// Render commands
		this->renderer.CmdBeginFrame(0.0f, 0.0f, 0.1f, 1.0f);

		this->renderer.CmdBindPipeline(Input::GetKeyDown(Key::One) ? 1 : 0);
		this->renderer.CmdUpdatePushConstant(Renderer::ShaderStage::Vertex, glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(0.0f, 1.0f, 0.0f)));
		for (uint32_t i = 0; i < this->meshCount; i++)
		{
			// What do you think these arguments denote:
			this->renderer.CmdBindTexture(textureIDs[i]);
			this->renderer.CmdDraw(i);
		}

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
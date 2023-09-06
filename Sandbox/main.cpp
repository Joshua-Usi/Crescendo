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
	float pMouseX = 0.0f, pMouseY = 0.0f, sens = 0.0005f;

	uint32_t meshCount = 0;
	bool uReleased = true;

	std::vector<uint32_t> textureIDs;
	std::vector<bool> isTransparent;

	Renderer renderer = {};

	int frame = 0;
	double lastTime = 0.0;
public:
	void OnStartup()
	{
		CVar::LoadConfigXML("config.xml");
		
		this->GetWindow()->SetSize(CVar::Get<int64_t>("ec_windowwidth"), CVar::Get<int64_t>("ec_windowheight"));
		this->GetWindow()->SetCursorLock(true);

		this->camera = Graphics::Camera(70.0f, this->GetWindow()->GetAspectRatio(), { 0.1f, 100000.0f });

		Renderer::BuilderInfo info;

		info.useValidationLayers = CVar::Get<bool>("rc_validationlayers");
		info.preferredDeviceType = Renderer::BuilderInfo::DeviceType::Discrete;
		info.appName = CVar::Get<std::string>("rc_appname");
		info.engineName = CVar::Get<std::string>("rc_enginename");
		info.window = this->GetWindow()->GetNative();
		info.windowExtent = { this->GetWindow()->GetWidth(), this->GetWindow()->GetHeight() };
		info.preferredPresentMode = Renderer::BuilderInfo::PresentMode::Mailbox;
		info.framesInFlight = CVar::Get<int64_t>("rc_framesinflight");
		info.vertexBufferBlockSize = std::powl(2, 24); // use 24 for sponza, 28 for rungholt
		info.descriptorBufferBlockSize = std::powl(2, 18); // 256KB
		info.msaaSamples = CVar::Get<int64_t>("rc_multisamples");

		this->renderer = Renderer::Create(info);

		Construct::Mesh skybox = Construct::SkyboxSphere(32, 32);
		IO::Model skyboxModel = {{{ skybox.vertices, skybox.normals, skybox.textureUVs, skybox.indices, "./assets/skybox-night.png", }}};

		std::vector<IO::Model> models =
		{
			IO::LoadOBJ("./assets/sponza/sponza.obj", "./assets/sponza/"),
			skyboxModel
		};

		std::unordered_map<std::string, uint32_t> seenTextures;
		std::unordered_map<std::string, bool> seenAlphas;
		this->meshCount = 0;
		uint32_t i = 0;
		for (const auto& model : models)
		{
			this->meshCount += model.meshes.size();
			
			for (const auto& mesh : model.meshes)
			{
				// Mesh upload
				this->renderer.UploadMesh(mesh.vertices, mesh.normals, mesh.textureUVs, mesh.indices);

				// Texture upload
				if (mesh.albedo.empty() || seenTextures.find(mesh.albedo) != seenTextures.end())
				{
					this->textureIDs.push_back(seenTextures[mesh.albedo]);
					this->isTransparent.push_back(seenAlphas[mesh.albedo]);
				}
				else
				{
					IO::Image image = IO::LoadImage(mesh.albedo);
					seenTextures[mesh.albedo] = i;
					this->textureIDs.push_back(seenTextures[mesh.albedo]);

					seenAlphas[mesh.albedo] = image.isTransparent;
					this->isTransparent.push_back(image.isTransparent);

					this->renderer.UploadTexture(image.pixels, image.width, image.height, image.channels, true);
					i++;
				}
			}
		}

		// Upload shaders (creates pipelines and descriptor sets)
		// Shader loading
		struct Shader { std::string name; Renderer::PipelineVariant variant; };
		std::vector<Shader> shaderList = {
			{"./shaders/compiled/mesh", Renderer::PipelineVariant() }, // Normal meshes
			{"./shaders/compiled/mesh", Renderer::PipelineVariant(Renderer::PipelineVariant::FillMode::Solid, true, false, Renderer::PipelineVariant::DepthFunc::Less, Renderer::PipelineVariant::CullMode::None) }, // Transparent meshes
			{"./shaders/compiled/skybox", Renderer::PipelineVariant(Renderer::PipelineVariant::FillMode::Solid, false, false) }, // Skybox
		};
		for (const auto& shader : shaderList)
		{
			this->renderer.UploadPipeline(
				Crescendo::Core::BinaryFile(shader.name + ".vert.spv").Open().Read(),
				Crescendo::Core::BinaryFile(shader.name + ".frag.spv").Open().Read(),
				shader.variant
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
		rotation.x += (float)dx * sens;
		rotation.y = std::clamp<float>(rotation.y + dy * sens, -Math::PI_2 + 0.01f, Math::PI_2 - 0.01f);
		this->camera.SetRotation(rotation);

		// Camera movement
		float velocity = 0.1f;
		glm::vec3 movement(0.0f, 0.0f, 0.0f);
		if (Input::GetKeyPressed(Key::R)) velocity = 10.0f;

		if (Input::GetKeyDown(Key::F11)) this->GetWindow()->SetFullScreen(!this->GetWindow()->IsFullScreen());

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
		struct Lighting { glm::vec4 lightColor, lightPosition; } lighting{ glm::vec4(0.75f, 0.75f, 1.0f, 1.0f), glm::vec4(0.0f, 10000.0f, 0.0f, 1.0f) };

		this->renderer.UpdateDescriptorSetData(0, 0, vp);
		this->renderer.UpdateDescriptorSetData(1, 0, lighting);
		this->renderer.UpdateDescriptorSetData(1, 1, glm::vec3(0.25f, 0.75f, 0.0f));

		this->renderer.UpdateDescriptorSetData(2, 0, vp);
		this->renderer.UpdateDescriptorSetData(3, 0, lighting);
		this->renderer.UpdateDescriptorSetData(3, 1, glm::vec3(0.25f, 0.75f, 0.0f));

		this->renderer.UpdateDescriptorSetData(4, 0, vp);

		// Render commands
		this->renderer.CmdBeginFrame(0.0f, 0.0f, 0.0f, 1.0f);

		this->renderer.CmdBindPipeline(2);
		this->renderer.CmdUpdatePushConstant(Renderer::ShaderStage::Vertex, glm::translate(glm::mat4(1.0f), this->camera.GetPosition()));
		this->renderer.CmdBindTexture(textureIDs[this->meshCount - 1]);
		this->renderer.CmdDraw(this->meshCount - 1);

		this->renderer.CmdBindPipeline(0);
		this->renderer.CmdUpdatePushConstant(Renderer::ShaderStage::Vertex, glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(0.0f, 1.0f, 0.0f)));
		for (uint32_t i = 0; i < this->meshCount - 1; i++)
		{
			if (this->isTransparent[i]) continue;
			this->renderer.CmdBindTexture(textureIDs[i]);
			this->renderer.CmdDraw(i);
		}

		this->renderer.CmdBindPipeline(1);
		this->renderer.CmdUpdatePushConstant(Renderer::ShaderStage::Vertex, glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(0.0f, 1.0f, 0.0f)));
		for (uint32_t i = 0; i < this->meshCount - 1; i++)
		{
			if (!this->isTransparent[i]) continue;
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
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

	std::vector<glm::mat4> meshTransforms;
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

		this->camera = Graphics::Camera(70.0f, this->GetWindow()->GetAspectRatio(), { 0.1f, 10000.0f });
		this->camera.SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));

		Renderer::BuilderInfo info;

		info.useValidationLayers = CVar::Get<bool>("rc_validationlayers");
		info.preferredDeviceType = Renderer::BuilderInfo::DeviceType::Discrete;
		info.appName = CVar::Get<std::string>("rc_appname");
		info.engineName = CVar::Get<std::string>("rc_enginename");
		info.window = this->GetWindow()->GetNative();
		info.windowExtent = { this->GetWindow()->GetWidth(), this->GetWindow()->GetHeight() };
		info.preferredPresentMode = Renderer::BuilderInfo::PresentMode::Mailbox;
		info.framesInFlight = CVar::Get<int64_t>("rc_framesinflight");
		info.vertexBufferBlockSize = std::powl(2, 28); // use 24 for sponza, 28 for rungholt
		info.descriptorBufferBlockSize = std::powl(2, 18); // 256KB
		info.msaaSamples = CVar::Get<int64_t>("rc_multisamples");

		this->renderer = Renderer::Create(info);

		Construct::Mesh skybox = Construct::SkyboxSphere(32, 32);
		IO::Model skyboxModel = {{{ skybox.vertices, skybox.normals, skybox.textureUVs, skybox.indices, "./assets/skybox-night.png" }}};

		std::vector<IO::Model> models =
		{
			IO::LoadGLTF("./assets/modern-sponza/modern-sponza.gltf", "./assets/modern-sponza/"),
			IO::LoadGLTF("./assets/sponza-curtains/sponza-curtains.gltf", "./assets/sponza-curtains/"),
			//IO::LoadOBJ("./assets/obj-sponza/sponza.obj", "./assets/obj-sponza/"),
			//IO::LoadGLTF("./assets/companion-cube/scene.gltf", "./assets/companion-cube/"),
			IO::LoadGLTF("./assets/tree/tree.gltf", "./assets/tree/"),
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
				meshTransforms.push_back(mesh.transform);

				// Texture upload
				if (mesh.diffuse.empty() || seenTextures.find(mesh.diffuse) != seenTextures.end())
				{
					this->textureIDs.push_back(seenTextures[mesh.diffuse]);
					this->isTransparent.push_back(seenAlphas[mesh.diffuse]);
				}
				else
				{
					IO::Image image = IO::LoadImage(mesh.diffuse);
					seenTextures[mesh.diffuse] = i;
					this->textureIDs.push_back(seenTextures[mesh.diffuse]);

					this->isTransparent.push_back(false);

					this->renderer.UploadTexture(image.pixels, image.width, image.height, image.channels, true);
					i++;
				}
			}
		}

		std::cout << "Loaded " << seenTextures.size() << " textures" << std::endl;

		// Upload shaders (creates pipelines and descriptor sets)
		// Shader loading
		struct Shader { std::string name; Renderer::PipelineVariant variant; };
		std::vector<Shader> shaderList = {
			{"./shaders/compiled/mesh", Renderer::PipelineVariant(Renderer::PipelineVariant::FillMode::Solid, true, true, Renderer::PipelineVariant::DepthFunc::Less, Renderer::PipelineVariant::CullMode::None) }, // Normal meshes
			{"./shaders/compiled/mesh", Renderer::PipelineVariant(Renderer::PipelineVariant::FillMode::Solid, true, false, Renderer::PipelineVariant::DepthFunc::Less, Renderer::PipelineVariant::CullMode::None) }, // Transparent meshes
			{"./shaders/compiled/skybox", Renderer::PipelineVariant(Renderer::PipelineVariant::FillMode::Solid, true, false) }, // Skybox
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
		float velocity = 0.01f;
		glm::vec3 movement(0.0f, 0.0f, 0.0f);
		if (Input::GetKeyPressed(Key::R)) velocity = 1.0f;

		float sinX = std::sin(rotation.x) * velocity, cosX = std::cos(rotation.x) * velocity;

		if (Input::GetKeyPressed(Key::W)) movement.x -= sinX, movement.z -= cosX;
		if (Input::GetKeyPressed(Key::S)) movement.x += sinX, movement.z += cosX;
		if (Input::GetKeyPressed(Key::A)) movement.x -= cosX, movement.z += sinX;
		if (Input::GetKeyPressed(Key::D)) movement.x += cosX, movement.z -= sinX;

		if (Input::GetKeyPressed(Key::Space)) movement.y += 1.0f * velocity;
		if (Input::GetKeyPressed(Key::ShiftLeft)) movement.y -= 1.0f * velocity;
		this->camera.MovePosition(movement);

		// Render prep
		struct Lighting { glm::vec4 lightColor, lightPosition, viewPosition; } lighting{
			glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
			glm::vec4(10000.0f * sinf(this->GetTime()), 10000.0f * cos(this->GetTime()), 0.0f, 1.0f),
			glm::vec4(this->camera.GetPosition(), 1.0f)
		};
		const glm::vec3 lightIntensities = glm::vec3(0.25f, 0.5f, 0.25f);

		this->renderer.UpdateDescriptorSetData(0, 0, this->camera.GetViewProjectionMatrix());
		this->renderer.UpdateDescriptorSetData(1, 0, lighting);
		this->renderer.UpdateDescriptorSetData(1, 1, lightIntensities);

		this->renderer.UpdateDescriptorSetData(2, 0, this->camera.GetViewProjectionMatrix());
		this->renderer.UpdateDescriptorSetData(3, 0, lighting);
		this->renderer.UpdateDescriptorSetData(3, 1, lightIntensities);

		this->renderer.UpdateDescriptorSetData(4, 0, this->camera.GetViewProjectionMatrix());

		// Render commands
		this->renderer.CmdBeginFrame(0.0f, 0.0f, 0.0f, 1.0f);

		this->renderer.CmdBindPipeline(2);
		this->renderer.CmdUpdatePushConstant(Renderer::ShaderStage::Vertex, glm::translate(glm::mat4(1.0f), -this->camera.GetPosition()));
		this->renderer.CmdBindTexture(textureIDs[this->meshCount - 1]);
		this->renderer.CmdDraw(this->meshCount - 1);

		this->renderer.CmdBindPipeline(0);		
		for (uint32_t i = 0; i < this->meshCount - 1; i++)
		{
			if (this->isTransparent[i]) continue;
			this->renderer.CmdUpdatePushConstant(Renderer::ShaderStage::Vertex, this->meshTransforms[i]);
			this->renderer.CmdBindTexture(textureIDs[i]);
			this->renderer.CmdDraw(i);
		}

		this->renderer.CmdBindPipeline(1);
		for (uint32_t i = 0; i < this->meshCount - 1; i++)
		{
			if (!this->isTransparent[i]) continue;
			this->renderer.CmdUpdatePushConstant(Renderer::ShaderStage::Vertex, this->meshTransforms[i]);
			this->renderer.CmdBindTexture(textureIDs[i]);
			this->renderer.CmdDraw(i);
		}

		this->renderer.CmdEndFrame();
		this->renderer.CmdPresentFrame();

		if (Input::GetKeyDown(Key::F11)) this->GetWindow()->SetFullScreen(!this->GetWindow()->IsFullScreen());
		if (Input::GetKeyDown(Key::Escape)) this->Exit();
	}
	void OnExit()
	{
		Renderer::Destroy(this->renderer);
	}
};

CrescendoRegisterApp(Sandbox);
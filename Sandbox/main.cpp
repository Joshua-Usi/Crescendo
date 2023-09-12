#include "Crescendo.hpp"

using namespace Crescendo::Engine;
typedef Crescendo::Renderer Renderer;
namespace Graphics = Crescendo::Graphics;
namespace Math = Crescendo::Math;
namespace IO = Crescendo::IO;

#include "Libraries/Algorithms/Algorithms.hpp"

#include "glm/gtx/common.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <map>

#include "CameraController.hpp"

class Sandbox : public Application
{
private:
	CameraController camera;

	uint32_t meshCount = 0;
	bool uReleased = true;

	std::vector<Crescendo::Algorithms::AABB> meshBounds;
	std::vector<glm::mat4> meshTransforms;
	std::vector<uint32_t> textureIDs;

	int frame = 0;
	double lastTime = 0.0;
	int actualDrawCount = 0;
public:
	void OnStartup()
	{
		this->GetWindow()->SetCursorLock(true);

		this->camera = CameraController(70.0f, this->GetWindow()->GetAspectRatio(), { 0.1f, 10000.0f });

		Construct::Mesh skybox = Construct::SkyboxSphere(32, 32);
		IO::Model skyboxModel = {{{ skybox.vertices, skybox.normals, skybox.textureUVs, skybox.indices, "./assets/skybox-night.png" }}};

		std::vector<IO::Model> models =
		{
			IO::LoadGLTF("./assets/modern-sponza/modern-sponza.gltf"),
			IO::LoadGLTF("./assets/sponza-curtains/sponza-curtains.gltf"),
			//IO::LoadOBJ("./assets/obj-sponza/sponza.obj"),
			//IO::LoadGLTF("./assets/companion-cube/scene.gltf"),
			IO::LoadGLTF("./assets/tree/tree.gltf"),
			skyboxModel
		};

		std::map<std::filesystem::path, uint32_t> seenTextures;
		this->meshCount = 0;
		uint32_t i = 0;
		for (const auto& model : models)
		{
			this->meshCount += model.meshes.size();
			
			for (const auto& mesh : model.meshes)
			{
				// Mesh upload
				this->renderer.renderer.UploadMesh(mesh.vertices, mesh.normals, mesh.textureUVs, mesh.indices);
				meshTransforms.push_back(mesh.transform);
				meshBounds.push_back(Crescendo::Algorithms::CalculateMeshBoundingAABB(mesh.vertices));

				// Texture upload
				if (mesh.diffuse.empty() || seenTextures.find(mesh.diffuse) != seenTextures.end())
				{
					this->textureIDs.push_back(seenTextures[mesh.diffuse]);
				}
				else
				{
					seenTextures[mesh.diffuse] = i;
					this->textureIDs.push_back(seenTextures[mesh.diffuse]);
					i++;
				}
			}
		}
		seenTextures.erase("");

		std::vector<std::filesystem::path> textureStrings(seenTextures.size());
		for (const auto& texture : seenTextures) textureStrings[texture.second] = texture.first;
		std::vector<IO::Image> images(textureStrings.size());
		for (uint32_t i = 0; i < textureStrings.size(); i++)
		{
			this->taskQueue.AddTask([&images, &textureStrings, i]() { images[i] = IO::LoadImage(textureStrings[i]); });
		}
		this->taskQueue.WaitTillFinished();
		for (auto& image : images) this->renderer.renderer.UploadTexture(image.pixels.get(), image.width, image.height, image.channels, false);

		// Upload shaders (creates pipelines and descriptor sets)
		// Shader loading
		struct Shader { std::string name; Renderer::PipelineVariant variant; };
		std::vector<Shader> shaderList = {
			{"./shaders/compiled/mesh", Renderer::PipelineVariant(Renderer::PipelineVariant::FillMode::Solid, true, true, Renderer::PipelineVariant::DepthFunc::Less, Renderer::PipelineVariant::CullMode::Back) }, // Single sides, opaque meshes
			{"./shaders/compiled/mesh", Renderer::PipelineVariant(Renderer::PipelineVariant::FillMode::Solid, true, true, Renderer::PipelineVariant::DepthFunc::Less, Renderer::PipelineVariant::CullMode::None) }, // Double sided, opaque meshes
			{"./shaders/compiled/mesh", Renderer::PipelineVariant(Renderer::PipelineVariant::FillMode::Solid, true, false, Renderer::PipelineVariant::DepthFunc::Less, Renderer::PipelineVariant::CullMode::Back) }, // Single sided, transparent meshes
			{"./shaders/compiled/mesh", Renderer::PipelineVariant(Renderer::PipelineVariant::FillMode::Solid, true, false, Renderer::PipelineVariant::DepthFunc::Less, Renderer::PipelineVariant::CullMode::None) }, // Double sided, transparent meshes
			{"./shaders/compiled/skybox", Renderer::PipelineVariant(Renderer::PipelineVariant::FillMode::Solid, true, false) }, // Skybox
		};
		for (const auto& shader : shaderList)
		{
			this->renderer.renderer.UploadPipeline(
				Crescendo::BinaryFile(shader.name + ".vert.spv").Open().Read(),
				Crescendo::BinaryFile(shader.name + ".frag.spv").Open().Read(),
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
			this->GetWindow()->SetName("Crescendo | FPS: " + std::to_string(this->frame) + " | Dc: " + std::to_string(this->meshCount) + "| aDc: " + std::to_string(this->actualDrawCount));
			this->frame = 0;
		}

		this->camera.Update();

		// Render prep
		struct Lighting { glm::vec4 lightColor, lightPosition, viewPosition; } lighting {
			glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
			glm::vec4(100.0f * sinf(this->GetTime()), 100.0f * cos(this->GetTime()), 0.0f, 1.0f),
			glm::vec4(this->camera.camera->GetPosition(), 1.0f)
		};
		const glm::vec3 lightIntensities = glm::vec3(0.25f, 0.5f, 0.25f);

		glm::mat4 viewProj = this->camera.camera->GetViewProjectionMatrix();

		for (uint32_t i = 0; i < 4; i++)
		{
			this->renderer.renderer.UpdateDescriptorSetData(i * 2, 0, viewProj);
			this->renderer.renderer.UpdateDescriptorSetData(i * 2 + 1, 0, lighting);
			this->renderer.renderer.UpdateDescriptorSetData(i * 2 + 1, 1, lightIntensities);
		}
		this->renderer.renderer.UpdateDescriptorSetData(8, 0, viewProj);

		this->actualDrawCount = 0;

		// Render commands
		this->renderer.renderer.CmdBeginFrame(0.0f, 0.0f, 0.0f, 1.0f);

		this->renderer.renderer.CmdBindPipeline(4);
		this->renderer.renderer.CmdUpdatePushConstant(Renderer::ShaderStage::Vertex, glm::translate(glm::mat4(1.0f), this->camera.camera->GetPosition()));
		this->renderer.renderer.CmdBindTexture(textureIDs[this->meshCount - 1]);
		this->renderer.renderer.CmdDraw(this->meshCount - 1);
		this->actualDrawCount++;

		this->renderer.renderer.CmdBindPipeline(0);
		for (uint32_t i = 0; i < this->meshCount - 1; i++)
		{
			this->actualDrawCount++;
			this->renderer.renderer.CmdUpdatePushConstant(Renderer::ShaderStage::Vertex, this->meshTransforms[i]);
			this->renderer.renderer.CmdBindTexture(textureIDs[i]);
			this->renderer.renderer.CmdDraw(i);
		}

		this->renderer.renderer.CmdEndFrame();
		this->renderer.renderer.CmdPresentFrame();

		if (Input::GetKeyDown(Key::F11)) this->GetWindow()->SetFullScreen(!this->GetWindow()->IsFullScreen());
		if (Input::GetKeyDown(Key::Escape)) this->Exit();
	}
	void OnExit()
	{

	}
};

CrescendoRegisterApp(Sandbox);
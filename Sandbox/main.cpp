#include "Crescendo.hpp"

using namespace Crescendo::Engine;
typedef Crescendo::Renderer Renderer;
namespace Graphics = Crescendo::Graphics;
namespace IO = Crescendo::IO;

#include <cs_std/graphics/algorithms.hpp>

#include "glm/gtx/common.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "CameraController.hpp"

#include <map>

class Sandbox : public Application
{
private:
	CameraController camera;
	Graphics::OrthographicCamera UICamera;
	Graphics::OrthographicCamera shadowMapCamera;

	uint32_t meshCount = 0;

	std::vector<cs_std::graphics::bounding_aabb> meshBounds;
	std::vector<glm::mat4> meshTransforms;

	struct TextureIDs { uint32_t diffuse, normal; };

	std::vector<TextureIDs> textureIDs;
	std::vector<bool> isTransparent, isDoubleSided, isShadowCasting;

	int frame = 0;
	double lastTime = 0.0;
public:
	void OnStartup()
	{

		this->GetWindow()->SetCursorLock(true);

		this->camera = CameraController(70.0f, this->GetWindow()->GetAspectRatio(), { 0.1f, 1000.0f });
		this->camera.camera.SetPosition(glm::vec3(0.0f, 1.0f, 0.0f));

		this->UICamera = Graphics::OrthographicCamera(
			glm::vec4(0.0f, this->GetWindow()->GetWidth(), this->GetWindow()->GetHeight(), 0.0f),
			glm::vec2(-1.0f, 1.0f)
		);
		this->UICamera.SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
		this->UICamera.SetRotation(glm::quat(0.0f, 1.0f, 0.0f, 0.0f));

		this->shadowMapCamera = Graphics::OrthographicCamera(
			glm::vec4(-20.0f, 20.0f, -20.0f, 20.0f),
			glm::vec2(-21.0f, 61.0f)
		);
			
		// Upload shaders (creates pipelines and descriptor sets)

		double now = this->GetTime(), now2 = now;

		struct Shader { std::string name; Renderer::PipelineVariants variants; };
		std::vector<Shader> shaderList = {
			{
				"./shaders/compiled/mesh",
				Renderer::PipelineVariants(
					{ Renderer::PipelineVariants::FillMode::Solid },
					{ true },
					{ true, false },
					{ Renderer::PipelineVariants::DepthFunc::Less },
					{ Renderer::PipelineVariants::CullMode::Back, Renderer::PipelineVariants::CullMode::None },
					{ Renderer::PipelineVariants::RenderPass::Default }
				)
			},
			{"./shaders/compiled/skybox", Renderer::PipelineVariants(Renderer::PipelineVariants::FillMode::Solid, true, false, Renderer::PipelineVariants::DepthFunc::Less, Renderer::PipelineVariants::CullMode::Back, Renderer::PipelineVariants::RenderPass::Default) }, // Skybox
			{"./shaders/compiled/shadow_map", Renderer::PipelineVariants(Renderer::PipelineVariants::FillMode::Solid, true, true, Renderer::PipelineVariants::DepthFunc::Less, Renderer::PipelineVariants::CullMode::Front, Renderer::PipelineVariants::RenderPass::Shadow) }, // Shadow map
			{"./shaders/compiled/ui", Renderer::PipelineVariants(Renderer::PipelineVariants::FillMode::Solid, false, false, Renderer::PipelineVariants::DepthFunc::Less, Renderer::PipelineVariants::CullMode::None, Renderer::PipelineVariants::RenderPass::Default) }, // UI
		};
		for (const auto& shader : shaderList)
		{
			this->renderer.renderer.UploadPipeline(
				cs_std::binary_file(shader.name + ".vert.spv").open().read(),
				cs_std::binary_file(shader.name + ".frag.spv").open().read(),
				shader.variants
			);
		}

		cs_std::console::log("Shader upload took", this->GetTime() - now, "s");
		now = this->GetTime();

		Construct::Mesh skybox = Construct::SkyboxSphere(32, 32);
		IO::Model skyboxModel = { {{ skybox.vertices, {}, skybox.textureUVs, {}, skybox.indices, "./assets/skybox.png"}} };

		Construct::Mesh quad = Construct::Quad();
		IO::Model quadModel = { {{ quad.vertices, {}, quad.textureUVs, {}, quad.indices}} };

		std::vector<IO::Model> models =
		{
			IO::LoadGLTF("./assets/modern-sponza/modern-sponza.gltf"),
			IO::LoadGLTF("./assets/sponza-curtains/sponza-curtains.gltf"),
			//IO::LoadGLTF("./assets/sponza-ivy/sponza-ivy.gltf"),
			//IO::LoadOBJ("./assets/obj-sponza/sponza.obj"),
			//IO::LoadGLTF("./assets/companion-cube/scene.gltf"),
			IO::LoadGLTF("./assets/tree/tree.gltf"),
			skyboxModel,
			quadModel
		};

		std::map<std::filesystem::path, uint32_t> seenTexturesDiffuse;
		std::map<std::filesystem::path, uint32_t> seenTexturesNormal;
		this->meshCount = 0;
		uint32_t i = 0, j = 0;
		for (const auto& model : models)
		{
			this->meshCount += model.meshes.size();
			
			for (const auto& mesh : model.meshes)
			{
				// Mesh upload
				std::vector<Renderer::ShaderAttribute> attributes;
				if (mesh.vertices.size() > 0) attributes.emplace_back(mesh.vertices, Renderer::ShaderAttributeFlag::Position);
				if (mesh.normals.size() > 0) attributes.emplace_back(mesh.normals, Renderer::ShaderAttributeFlag::Normal);
				if (mesh.tangents.size() > 0) attributes.emplace_back(mesh.tangents, Renderer::ShaderAttributeFlag::Tangent);
				if (mesh.textureUVs.size() > 0) attributes.emplace_back(mesh.textureUVs, Renderer::ShaderAttributeFlag::TexCoord_0);

				this->renderer.renderer.UploadMesh(attributes, mesh.indices);

				meshTransforms.push_back(mesh.transform);
				meshBounds.push_back(cs_std::graphics::bounding_aabb(mesh.vertices).transform(mesh.transform));
				isTransparent.push_back(mesh.isTransparent);
				isDoubleSided.push_back(mesh.isDoubleSided);
				isShadowCasting.push_back(true);

				// Texture upload
				if (!mesh.diffuse.empty() && seenTexturesDiffuse.find(mesh.diffuse) == seenTexturesDiffuse.end())
				{
					seenTexturesDiffuse[mesh.diffuse] = i;
					i++;
				}
				if (!mesh.normal.empty() && seenTexturesNormal.find(mesh.normal) == seenTexturesNormal.end())
				{
					seenTexturesNormal[mesh.normal] = j;
					j++;
				}
				this->textureIDs.push_back(TextureIDs(seenTexturesDiffuse[mesh.diffuse], seenTexturesNormal[mesh.normal]));
			}
		}
		seenTexturesDiffuse.erase("");
		seenTexturesNormal.erase("");
		for (auto& textureIDs : this->textureIDs) textureIDs.normal += seenTexturesDiffuse.size();

		std::vector<std::filesystem::path> textureStrings(seenTexturesDiffuse.size() + seenTexturesNormal.size());
		for (const auto& texture : seenTexturesDiffuse) textureStrings[texture.second] = texture.first;
		for (const auto& texture : seenTexturesNormal) textureStrings[seenTexturesDiffuse.size() + texture.second] = texture.first;

		cs_std::console::log("Mesh upload took", this->GetTime() - now, "s");
		now = this->GetTime();

		std::vector<IO::Image> images(textureStrings.size());

		std::atomic<uint32_t> finishedTasks = 0;
		uint32_t last = 0;

		for (uint32_t i = 0; i < textureStrings.size(); i++)
		{
			this->taskQueue.push_back([&images, &textureStrings, i, &finishedTasks]() { images[i] = IO::LoadImage(textureStrings[i]); finishedTasks++; });
		}
		
		while (!this->taskQueue.finished())
		{
			if (finishedTasks != last)
			{
				for (uint32_t i = 0; i < finishedTasks - last; i++) cs_std::console::raw("#");
				last = finishedTasks;
			}
		}
		cs_std::console::raw('\n');

		this->taskQueue.sleep();

		cs_std::console::log("Texture decode took", this->GetTime() - now, "s");
		now = this->GetTime();

		for (auto& image : images) this->renderer.renderer.UploadTexture(image.pixels.get(), image.width, image.height, image.channels, false);

		cs_std::console::log("Texture upload took", this->GetTime() - now, "s");
		cs_std::console::log("Startup took", this->GetTime() - now2, "s");
	}
	void OnUpdate(double dt)
	{
		this->camera.Update();

		float currentTime = this->GetTime<float>() / 2.5f;
		this->shadowMapCamera.SetPosition(glm::vec3(0.0f, std::cosf(currentTime) * 40.0f, std::sinf(currentTime) * 40.0f));
		this->shadowMapCamera.LookAt(glm::vec3(0.0f, 0.0f, 0.0f));

		// Render prep
		const glm::mat4 projections[2] = { this->camera.camera.GetViewProjectionMatrix(), this->shadowMapCamera.GetViewProjectionMatrix() };
		const glm::vec4 lightingPositions[2] = { glm::vec4(this->shadowMapCamera.GetPosition(), 1.0f), glm::vec4(this->camera.camera.GetPosition(), 1.0f) };
		const glm::vec3 lightIntensities = glm::vec3(0.2f, 0.5f, 0.3f);
		
		this->renderer.renderer.UpdateDescriptorSetData(0, 0, projections);
		this->renderer.renderer.UpdateDescriptorSetData(0, 1, lightingPositions);
		this->renderer.renderer.UpdateDescriptorSetData(1, 0, lightIntensities);
		this->renderer.renderer.UpdateDescriptorSetData(2, 0, this->camera.camera.GetViewProjectionMatrix());
		this->renderer.renderer.UpdateDescriptorSetData(3, 0, this->shadowMapCamera.GetViewProjectionMatrix());
		this->renderer.renderer.UpdateDescriptorSetData(4, 0, this->UICamera.GetViewProjectionMatrix());

		uint32_t actualDrawCount = 0;

		// Render commands
		this->renderer.renderer.CmdBeginFrame();
		// Shadow render pass
		{
			this->renderer.renderer.CmdBeginRenderPass(1);
			cs_std::graphics::frustum frustum(this->shadowMapCamera.GetViewProjectionMatrix());
			this->renderer.renderer.CmdBindPipeline(5);

			for (uint32_t i = 0; i < this->meshCount - 2; i++)
			{
				if (!frustum.intersects(this->meshBounds[i])) continue;
				if (!this->isShadowCasting[i]) continue;

				actualDrawCount++;
				this->renderer.renderer.CmdUpdatePushConstant(Renderer::ShaderStage::Vertex, this->meshTransforms[i]);
				this->renderer.renderer.CmdDraw(i);
			}
			this->renderer.renderer.CmdEndRenderPass();
		}
		// Main render pass
		{
			this->renderer.renderer.CmdBeginRenderPass(0, 0.0f, 0.0f, 0.0f, 1.0f);

			this->renderer.renderer.CmdBindPipeline(4);
			this->renderer.renderer.CmdUpdatePushConstant(Renderer::ShaderStage::Vertex, glm::translate(glm::mat4(1.0f), this->camera.camera.GetPosition()));
			this->renderer.renderer.CmdBindTexture(1, textureIDs[this->meshCount - 2].diffuse);
			this->renderer.renderer.CmdDraw(this->meshCount - 2);
			actualDrawCount++;

			cs_std::graphics::frustum frustum(this->camera.camera.GetViewProjectionMatrix());

			for (uint32_t i = 0; i < this->meshCount - 2; i++)
			{
				if (!frustum.intersects(this->meshBounds[i])) continue;

				if (!isTransparent[i] && !isDoubleSided[i]) this->renderer.renderer.CmdBindPipeline(0);
				if (!isTransparent[i] && isDoubleSided[i]) this->renderer.renderer.CmdBindPipeline(1);
				if (isTransparent[i] && !isDoubleSided[i]) this->renderer.renderer.CmdBindPipeline(2);
				if (isTransparent[i] && isDoubleSided[i]) this->renderer.renderer.CmdBindPipeline(3);

				actualDrawCount++;
				this->renderer.renderer.CmdUpdatePushConstant(Renderer::ShaderStage::Vertex, this->meshTransforms[i]);
				this->renderer.renderer.CmdBindTexture(2, textureIDs[i].diffuse);
				this->renderer.renderer.CmdBindTexture(3, textureIDs[i].normal);
				this->renderer.renderer.CmdBindTexture(4, Renderer::SHADOW_MAP_ID);

				this->renderer.renderer.CmdDraw(i);
			}

			this->renderer.renderer.CmdBindPipeline(6);

			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(200.0f, -200.0f, 0.0f));
			model = glm::scale(model, glm::vec3(400.0f, 400.0f, 1.0f));

			this->renderer.renderer.CmdUpdatePushConstant(Renderer::ShaderStage::Vertex, model);
			this->renderer.renderer.CmdBindTexture(1, Renderer::SHADOW_MAP_ID);

			this->renderer.renderer.CmdDraw(this->meshCount - 1);
			actualDrawCount++;

			this->renderer.renderer.CmdEndRenderPass();
		}
		this->renderer.renderer.CmdEndFrame();
		this->renderer.renderer.CmdPresentFrame();

		// Frame counter
		frame++;
		if (this->GetTime() - this->lastTime >= 1.0)
		{
			this->lastTime = this->GetTime();
			this->GetWindow()->SetName(	"Crescendo | FPS: " + std::to_string(this->frame) +
										" | M: " + std::to_string(this->meshCount) +
										" | Dc: " + std::to_string(actualDrawCount));
			this->frame = 0;
		}

		if (Input::GetKeyDown(Key::F11)) this->GetWindow()->SetFullScreen(!this->GetWindow()->IsFullScreen());
		if (Input::GetKeyDown(Key::Escape)) this->Exit();
	}
	void OnExit()
	{

	}
};

CrescendoRegisterApp(Sandbox);
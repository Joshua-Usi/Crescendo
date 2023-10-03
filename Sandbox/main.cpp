#include "Crescendo.hpp"

using namespace Crescendo::Engine;
typedef Crescendo::Renderer Renderer;
namespace Graphics = Crescendo::Graphics;
namespace IO = Crescendo::IO;

#include "cs_std/random.hpp"

#include "Libraries/Algorithms/Algorithms.hpp"

#include "glm/gtx/common.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "CameraController.hpp"

class Sandbox : public Application
{
private:
	CameraController camera;
	Graphics::OrthographicCamera UICamera;
	Graphics::PerspectiveCamera shadowMapCamera;

	uint32_t meshCount = 0;
	bool uReleased = true;

	std::vector<Crescendo::Algorithms::BoundingAABB> meshBounds;
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
		this->camera.camera->SetPosition(glm::vec3(0.0f, 1.0f, 0.0f));

		this->UICamera = Graphics::OrthographicCamera(
			glm::vec4(0.0f, this->GetWindow()->GetWidth(), this->GetWindow()->GetHeight(), 0.0f),
			glm::vec2(-1.0f, 1.0f)
		);
		this->UICamera.SetPosition(glm::vec3(0.0f, 0.0f, 1.0f));
		this->UICamera.SetRotation(glm::quat(0.0f, 1.0f, 0.0f, 0.0f));

		this->shadowMapCamera = Graphics::PerspectiveCamera(80.0f, 1.0f, { 1.0f, 100.0f });
		this->shadowMapCamera.SetRotation(glm::vec3(-std::numbers::pi / 2, std::numbers::pi / 2, 0));
			
		// Upload shaders (creates pipelines and descriptor sets)
		// Shader loading
		struct Shader { std::string name; std::vector<Renderer::PipelineVariant> variants; };
		std::vector<Shader> shaderList = {
			{
				"./shaders/compiled/mesh",
				{
					Renderer::PipelineVariant(Renderer::PipelineVariant::FillMode::Solid, true, true, Renderer::PipelineVariant::DepthFunc::Less, Renderer::PipelineVariant::CullMode::Back), // Single sided, opaque meshes
					Renderer::PipelineVariant(Renderer::PipelineVariant::FillMode::Solid, true, true, Renderer::PipelineVariant::DepthFunc::Less, Renderer::PipelineVariant::CullMode::None), // Double sided, opaque meshes
					Renderer::PipelineVariant(Renderer::PipelineVariant::FillMode::Solid, true, false, Renderer::PipelineVariant::DepthFunc::Less, Renderer::PipelineVariant::CullMode::Back), // Single sided, transparent meshes
					Renderer::PipelineVariant(Renderer::PipelineVariant::FillMode::Solid, true, false, Renderer::PipelineVariant::DepthFunc::Less, Renderer::PipelineVariant::CullMode::None) // Double sided, transparent meshes
				}
			},
			{"./shaders/compiled/skybox", { Renderer::PipelineVariant(Renderer::PipelineVariant::FillMode::Solid, true, false) } }, // Skybox
			{"./shaders/compiled/shadow_map", { Renderer::PipelineVariant(Renderer::PipelineVariant::FillMode::Solid, true, true, Renderer::PipelineVariant::DepthFunc::LessEqual, Renderer::PipelineVariant::CullMode::None, Renderer::PipelineVariant::RenderPass::Shadow) } }, // Shadow map
			//{"./shaders/compiled/ui", Renderer::PipelineVariant(Renderer::PipelineVariant::FillMode::Solid, false, false, Renderer::PipelineVariant::DepthFunc::Less, Renderer::PipelineVariant::CullMode::None) }, // UI
		};
		for (const auto& shader : shaderList)
		{
			this->renderer.renderer.UploadPipeline(
				Crescendo::BinaryFile(shader.name + ".vert.spv").Open().Read(),
				Crescendo::BinaryFile(shader.name + ".frag.spv").Open().Read(),
				shader.variants
			);
		}

		Construct::Mesh skybox = Construct::SkyboxSphere(32, 32);
		IO::Model skyboxModel = { {{ skybox.vertices, {}, skybox.textureUVs, {}, skybox.indices, "./assets/skybox.png"}} };

		std::vector<IO::Model> models =
		{
			IO::LoadGLTF("./assets/modern-sponza/modern-sponza.gltf"),
			IO::LoadGLTF("./assets/sponza-curtains/sponza-curtains.gltf"),
			//IO::LoadGLTF("./assets/sponza-ivy/sponza-ivy.gltf"),
			//IO::LoadOBJ("./assets/obj-sponza/sponza.obj"),
			//IO::LoadGLTF("./assets/companion-cube/scene.gltf"),
			IO::LoadGLTF("./assets/tree/tree.gltf"),
			skyboxModel,
			//quadModel,
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
				attributes.push_back({ mesh.vertices, Renderer::ShaderAttributeFlag::Position });
				if (mesh.normals.size() > 0) attributes.push_back({ mesh.normals, Renderer::ShaderAttributeFlag::Normal });
				if (mesh.tangents.size() > 0) attributes.push_back({ mesh.tangents, Renderer::ShaderAttributeFlag::Tangent });
				attributes.push_back({ mesh.textureUVs, Renderer::ShaderAttributeFlag::TexCoord_0 });

				this->renderer.renderer.UploadMesh(attributes, mesh.indices);

				meshTransforms.push_back(mesh.transform);
				meshBounds.push_back(Crescendo::Algorithms::CalculateMeshBoundingAABB(mesh.vertices).Transform(mesh.transform));
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

		std::vector<IO::Image> images(textureStrings.size());
		for (uint32_t i = 0; i < textureStrings.size(); i++)
		{
			this->taskQueue.AddTask([&images, &textureStrings, i]() { images[i] = IO::LoadImage(textureStrings[i]); });
		}
		this->taskQueue.WaitTillFinished();
		for (auto& image : images) this->renderer.renderer.UploadTexture(image.pixels.get(), image.width, image.height, image.channels, false);
	}
	void OnUpdate(double dt)
	{
		this->camera.Update();

		float currentTime = this->GetTime<float>() / 5.0f;
		this->shadowMapCamera.SetPosition(glm::vec3(std::cosf(currentTime) * 6.0f, 30.0f, std::sinf(currentTime) * 6.0f));

		// Render prep
		struct data { glm::mat4 viewProj, lightSpace; } vertex {
			this->camera.camera->GetViewProjectionMatrix(),
			//this->camera.camera->GetViewProjectionMatrix()
			this->shadowMapCamera.GetViewProjectionMatrix()
		};
		struct VSLighting {glm::vec4 lightPosition, viewPosition; } vsLighting {
			glm::vec4(this->shadowMapCamera.GetPosition(), 1.0f),
			glm::vec4(this->camera.camera->GetPosition(), 1.0f)
		};
		const glm::vec3 lightIntensities = glm::vec3(0.2f, 0.5f, 0.3f);
		
		this->renderer.renderer.UpdateDescriptorSetData(0, 0, vertex);
		this->renderer.renderer.UpdateDescriptorSetData(0, 1, vsLighting);
		this->renderer.renderer.UpdateDescriptorSetData(1, 0, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
		this->renderer.renderer.UpdateDescriptorSetData(1, 1, lightIntensities);
		this->renderer.renderer.UpdateDescriptorSetData(2, 0, this->camera.camera->GetViewProjectionMatrix());
		this->renderer.renderer.UpdateDescriptorSetData(3, 0, this->shadowMapCamera.GetViewProjectionMatrix());
		//this->renderer.renderer.UpdateDescriptorSetData(3, 0, this->camera.camera->GetViewProjectionMatrix());

		uint32_t actualDrawCount = 0;

		// Render commands
		this->renderer.renderer.CmdBeginFrame();
		// Shadow render pass
		{
			this->renderer.renderer.CmdBeginRenderPass(1);
			Crescendo::Algorithms::Frustum frustum = Crescendo::Algorithms::GetFrustum(this->shadowMapCamera.GetViewProjectionMatrix());
			this->renderer.renderer.CmdBindPipeline(5);

			for (uint32_t i = 0; i < this->meshCount - 1; i++)
			{
				if (!Crescendo::Algorithms::IsInFrustum(frustum, this->meshBounds[i])) continue;
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
			this->renderer.renderer.CmdUpdatePushConstant(Renderer::ShaderStage::Vertex, glm::translate(glm::mat4(1.0f), this->camera.camera->GetPosition()));
			this->renderer.renderer.CmdBindTexture(1, textureIDs[this->meshCount - 1].diffuse);
			this->renderer.renderer.CmdDraw(this->meshCount - 1);
			actualDrawCount++;

			Crescendo::Algorithms::Frustum frustum = Crescendo::Algorithms::GetFrustum(this->camera.camera->GetViewProjectionMatrix());

			for (uint32_t i = 0; i < this->meshCount - 1; i++)
			{
				if (!Crescendo::Algorithms::IsInFrustum(frustum, this->meshBounds[i])) continue;

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

			/*this->renderer.renderer.CmdBindPipeline(5);

			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(100.0f, 100.0f, 0.0f));
			model = glm::scale(model, glm::vec3(200.0f, 200.0f, 1.0f));

			for (uint32_t i = 0; i < 2; i++)
			{
				for (uint32_t j = 0; j < 2; j++)
				{
					this->renderer.renderer.CmdUpdatePushConstant(Renderer::ShaderStage::Vertex, model);
					this->renderer.renderer.CmdBindTexture(textureIDs[this->meshCount - 1]);
					this->renderer.renderer.CmdDraw(this->meshCount - 1);
					actualDrawCount++;
					model = glm::translate(model, glm::vec3(1.0f, 0.0f, 0.0f));
				}
				model = glm::translate(model, glm::vec3(-2.0f, 1.0f, 0.0f));
			}*/

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
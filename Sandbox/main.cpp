#define CS_SHOW_TIMINGS
#include "Crescendo.hpp"

using namespace Crescendo::Engine;
namespace Graphics = Crescendo::Graphics;
namespace IO = Crescendo::IO;

#include "cs_std/graphics/algorithms.hpp"
#include "cs_std/packed_vector.hpp"

#include "glm/gtx/common.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "CameraController.hpp"

#include <map>

#include "glfw/glfw3.h"

#include "Rendering/VulkanInstance/VulkanInstance.hpp"

//struct Transform
//{
//	glm::mat4 transform;
//	Transform(const glm::mat4& transform) : transform(transform) {}
//
//	operator glm::mat4&() { return this->transform; }
//};
//
struct ModelData
{
	cs_std::graphics::bounding_aabb bounds;
	// textureID and normalID is the index of the texture in the textures array
	// meshID is the index of the mesh to read from the SSBO
	uint32_t textureID, normalID; /*meshID*/
	bool isTransparent, isDoubleSided, isShadowCasting;
};

class Sandbox : public Application
{
private:
	CameraController camera;
	Graphics::OrthographicCamera UICamera;
	Graphics::OrthographicCamera shadowMapCamera;

	std::vector<ModelData> modelData;
	//std::vector<Entity> entities;

	int frame = 0;
	double lastTime = 0.0;

	Crescendo::VulkanInstance renderer;
public:
	void OnStartup()
	{
		this->GetWindow()->SetCursorLock(true);

		this->camera = CameraController(70.0f, this->GetWindow()->GetAspectRatio(), { 0.1f, 1000.0f });
		this->camera.camera.SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
		this->UICamera = Graphics::OrthographicCamera(
			glm::vec4(0.0f, this->GetWindow()->GetWidth(), this->GetWindow()->GetHeight(), 0.0f),
			glm::vec2(-1.0f, 1.0f)
		);
		this->UICamera.SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
		this->UICamera.SetRotation(glm::quat(0.0f, 1.0f, 0.0f, 0.0f));

		this->shadowMapCamera = Graphics::OrthographicCamera(
			glm::vec4(-27.5f, 27.5f, -27.5f, 27.5f),
			glm::vec2(0.0f, 100.0f)
		);

		/* ---------------------------------------------------------------- 0 - Vulkan setup ---------------------------------------------------------------- */

		Crescendo::VulkanInstanceSpecification spec {
			.enableValidationLayers = CVar::Get<bool>("rc_validationlayers"),
			.appName = CVar::Get<std::string>("rc_appname"),
			.engineName = CVar::Get<std::string>("rc_enginename"),
			.window = this->GetWindow()->GetNative(),
			.descriptorSetsPerPool = static_cast<uint32_t>(CVar::Get<int64_t>("rc_descriptorsetsperpool")),
			.framesInFlight = static_cast<uint32_t>(CVar::Get<int64_t>("rc_framesinflight")),
			.anisotropicSamples = static_cast<uint32_t>(CVar::Get<int64_t>("rc_anisotropicsamples")),
			.multisamples = static_cast<uint32_t>(CVar::Get<int64_t>("rc_multisamples")),
			.renderScale = CVar::Get<float>("rc_renderscale")
		};

		this->renderer = Crescendo::VulkanInstance(spec);

		/* ---------------------------------------------------------------- 1.0 - Shader data ---------------------------------------------------------------- */
		struct Shader { std::string name; Crescendo::Vulkan::PipelineVariants variants; };

		std::vector<Shader> shaderList {
			{ "./shaders/compiled/mesh", Crescendo::Vulkan::PipelineVariants::GetDefaultVariant(this->renderer.renderPasses[0]) },
			{ "./shaders/compiled/mesh-unlit", Crescendo::Vulkan::PipelineVariants::GetDefaultVariant(this->renderer.renderPasses[0])},
			{ "./shaders/compiled/skybox", Crescendo::Vulkan::PipelineVariants::GetSkyboxVariant(this->renderer.renderPasses[0]) },
			{ "./shaders/compiled/shadow_map", Crescendo::Vulkan::PipelineVariants::GetShadowVariant(this->renderer.renderPasses[2]) },
			{ "./shaders/compiled/ui", Crescendo::Vulkan::PipelineVariants::GetUIVariant(this->renderer.renderPasses[0]) },
			{ "./shaders/compiled/post_processing", Crescendo::Vulkan::PipelineVariants::GetPostProcessingVariant(this->renderer.renderPasses[1]) }
		};

		for (const auto& shader : shaderList)
		{
			this->renderer.pipelines.insert(this->renderer.device.CreatePipelines(
				cs_std::binary_file(shader.name + ".vert.spv").open().read(),
				cs_std::binary_file(shader.name + ".frag.spv").open().read(),
				shader.variants
			));
		}

		/* ---------------------------------------------------------------- 1.1 - Descriptor Data ---------------------------------------------------------------- */

		this->renderer.pipelines[0].CreateDescriptorSets(0, 3); // One per frame in flight (fif)
		this->renderer.pipelines[0].CreateDescriptorSet(1);
		this->renderer.pipelines[1].CreateDescriptorSets(0, 3); // One per fif
		this->renderer.pipelines[2].CreateDescriptorSets(0, 3); // One per fif
		this->renderer.pipelines[3].CreateDescriptorSets(0, 3); // One per fif
		this->renderer.pipelines[4].CreateDescriptorSets(0, 3); // One per fif

		this->renderer.pipelines[0].UpdateDescriptorData(0, 1, 0, glm::vec3(0.3f, 0.4f, 0.3f));

		/* ---------------------------------------------------------------- 1.2 - Mesh data ---------------------------------------------------------------- */

		cs_std::graphics::model skyboxModel {};
		{
			Construct::Mesh skybox = Construct::SkyboxSphere(32, 32);
			cs_std::graphics::mesh skyboxMesh {};
			skyboxMesh.indices = skybox.indices;
			skyboxMesh.add_attribute(cs_std::graphics::Attribute::POSITION, skybox.vertices);
			skyboxMesh.add_attribute(cs_std::graphics::Attribute::TEXCOORD_0, skybox.textureUVs);
			cs_std::graphics::mesh_attributes skyboxAttributes {};
			skyboxAttributes.diffuse = "./assets/skybox.png";

			skyboxModel.meshes.push_back(skyboxMesh);
			skyboxModel.meshAttributes.push_back(skyboxAttributes);
		}
		
		cs_std::graphics::model quadModel {};
		{
			Construct::Mesh quad = Construct::Quad();
			cs_std::graphics::mesh quadMesh {};
			quadMesh.indices = quad.indices;
			quadMesh.add_attribute(cs_std::graphics::Attribute::POSITION, quad.vertices);
			quadMesh.add_attribute(cs_std::graphics::Attribute::TEXCOORD_0, quad.textureUVs);
			cs_std::graphics::mesh_attributes quadAttributes {};

			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(100.0f, -100.0f, 0.0f));
			model = glm::scale(model, glm::vec3(200.0f, 200.0f, 1.0f));

			quadAttributes.transform = model;

			quadModel.meshes.push_back(quadMesh);
			quadModel.meshAttributes.push_back(quadAttributes);
		}

		std::vector<cs_std::graphics::model> models =
		{
			//IO::LoadGLTF("./assets/tristan/TRISTANSEXY.gltf"),
			IO::LoadGLTF("./assets/modern-sponza/modern-sponza.gltf"),
			IO::LoadGLTF("./assets/sponza-curtains/sponza-curtains.gltf"),
			//IO::LoadGLTF("./assets/sponza-ivy/sponza-ivy.gltf"),
			//IO::LoadGLTF("./assets/sponza-candles/sponza-candles.gltf"),
			//IO::LoadOBJ("./assets/obj-sponza/sponza.obj"),
			//IO::LoadGLTF("./assets/companion-cube/scene.gltf"),
			IO::LoadGLTF("./assets/tree/tree.gltf"),
			//IO::LoadGLTF("./assets/chair/chair.gltf"),
			skyboxModel,
			quadModel
		};

		uint32_t modelIndex = 0;
		uint32_t textureIndex = 0;
		std::map<std::filesystem::path, uint32_t> seenTextures;

		uint32_t indexCount = 0;

		for (auto& model : models)
		{
			for (uint32_t i = 0; i < model.meshes.size(); i++)
			{
				auto& mesh = model.meshes[i];
				auto& attributes = model.meshAttributes[i];

				if (!mesh.has_attribute(cs_std::graphics::Attribute::TANGENT)) cs_std::graphics::generate_tangents(mesh);

				indexCount += mesh.get_attribute(cs_std::graphics::Attribute::POSITION).data.size();

				this->renderer.meshes.insert(this->renderer.UploadMesh(mesh));

				if (!attributes.diffuse.empty() && seenTextures.find(attributes.diffuse) == seenTextures.end())
				{
					seenTextures[attributes.diffuse] = textureIndex;
					textureIndex++;
				}
				if (!attributes.normal.empty() && seenTextures.find(attributes.normal) == seenTextures.end())
				{
					seenTextures[attributes.normal] = textureIndex;
					textureIndex++;
				}

				this->modelData.emplace_back(
					cs_std::graphics::bounding_aabb(mesh.get_attribute(cs_std::graphics::Attribute::POSITION).data).transform(attributes.transform),
					seenTextures[attributes.diffuse] + 2, seenTextures[attributes.normal] + 2,
					attributes.isTransparent, attributes.isDoubleSided, true
				);

				// Fill the model matrix buffer
				for (uint32_t j = 0; j < this->renderer.specs.framesInFlight; j++)
				{
					memcpy(static_cast<char*>(this->renderer.ssbo[j].buffer.mPtr) + sizeof(glm::mat4) * modelIndex, &attributes.transform, sizeof(glm::mat4));
				}
				modelIndex++;
			}
		}

		cs_std::console::log("Total scene indices:", indexCount);

		/* ---------------------------------------------------------------- 1.3 - Texture Data ---------------------------------------------------------------- */

		seenTextures.erase("");
		std::vector<std::filesystem::path> textureStrings(seenTextures.size());
		for (const auto& texture : seenTextures) textureStrings[texture.second] = texture.first;

		std::vector<cs_std::image> images(textureStrings.size());

		uint32_t last = 0;
		for (uint32_t i = 0; i < textureStrings.size(); i++)
		{
			this->taskQueue.push_back([&images, &textureStrings, i]() { images[i] = IO::LoadImage(textureStrings[i]); });
		}
		
		while (!this->taskQueue.finished())
		{
			uint32_t local = textureStrings.size() - this->taskQueue.size();
			for (uint32_t i = 0; i < local - last; i++) cs_std::console::raw("#");
			last = local;
		}
		cs_std::console::raw('\n');
		this->taskQueue.sleep();

		for (auto& image : images) this->renderer.textures.insert(this->renderer.UploadTexture(image, false));
	}
	void OnUpdate(double dt)
	{
		/* ---------------------------------------------------------------- Game update ---------------------------------------------------------------- */

		this->camera.Update();

		float currentTime = this->GetTime<float>() / 20.0f - 2.0f;
		this->shadowMapCamera.SetPosition(glm::vec3(std::sinf(currentTime) * 75.0f, std::cosf(currentTime) * 75.0f, 0.0f));
		this->shadowMapCamera.LookAt(glm::vec3(0.0f, 0.0f, 0.0f));

		/* ---------------------------------------------------------------- Render preparation ---------------------------------------------------------------- */

		Crescendo::Vulkan::Pipelines& defaultPipeline = this->renderer.pipelines[0];
		Crescendo::Vulkan::Pipelines& unlitPipeline = this->renderer.pipelines[1];
		Crescendo::Vulkan::Pipelines& skyboxPipeline = this->renderer.pipelines[2];
		Crescendo::Vulkan::Pipelines& shadowPipeline = this->renderer.pipelines[3];
		Crescendo::Vulkan::Pipelines& uiPipeline = this->renderer.pipelines[4];
		Crescendo::Vulkan::Pipelines & postProcessingPipeline = this->renderer.pipelines[5];

		// Render prep
		const glm::mat4 projections[2] { this->camera.camera.GetViewProjectionMatrix(), this->shadowMapCamera.GetViewProjectionMatrix() };
		const glm::vec4 lightingPositions[2] { glm::vec4(this->shadowMapCamera.GetPosition(), 1.0f), glm::vec4(this->camera.camera.GetPosition(), 1.0f) };

		// Arguments in order: set index, set, binding, data
		defaultPipeline.UpdateDescriptorData(this->renderer.frameIndex, 0, 0, projections);
		defaultPipeline.UpdateDescriptorData(this->renderer.frameIndex, 0, 1, lightingPositions);
		unlitPipeline.UpdateDescriptorData(this->renderer.frameIndex, 0, 0, projections[0]);
		skyboxPipeline.UpdateDescriptorData(this->renderer.frameIndex, 0, 0, glm::translate(this->camera.camera.GetViewProjectionMatrix(), this->camera.camera.GetPosition()));
		shadowPipeline.UpdateDescriptorData(this->renderer.frameIndex, 0, 0, projections[1]);
		uiPipeline.UpdateDescriptorData(this->renderer.frameIndex, 0, 0, this->UICamera.GetViewProjectionMatrix());

		/* ---------------------------------------------------------------- Render commands ---------------------------------------------------------------- */

		Crescendo::Vulkan::RenderCommandQueue& cur = this->renderer.GetCurrentRenderCommandQueue();
		Crescendo::Vulkan::GraphicsCommandQueue& cmd = cur.cmd;

		cmd.WaitCompletion();
		cmd.Reset();

		uint32_t currentImage = this->renderer.swapchain.AcquireNextImage(cur.presentReady);
		if (this->renderer.swapchain.NeedsRecreation())
		{
			this->renderer.CreateSwapchain();
			currentImage = this->renderer.swapchain.AcquireNextImage(cur.presentReady);
		}
		const Crescendo::Vulkan::Framebuffer& framebuffer = this->renderer.framebuffers[currentImage];
		const Crescendo::Vulkan::Framebuffer& offScreen = this->renderer.framebuffers[this->renderer.offscreen.framebufferIndex];
		const Crescendo::Vulkan::Framebuffer& shadowFramebuffer = this->renderer.framebuffers[this->renderer.shadowMap.framebufferIndex];

		cmd.Begin();
			// Shadowmap pass
			cmd.DynamicStateSetViewport(shadowFramebuffer.GetViewport());
			cmd.DynamicStateSetScissor(shadowFramebuffer.GetScissor());
			cmd.BeginRenderPass(shadowFramebuffer.renderPass, shadowFramebuffer, shadowFramebuffer.GetScissor(), { Crescendo::Vulkan::Create::DefaultDepthClear() });
			{
				cs_std::graphics::frustum frustum(this->shadowMapCamera.GetViewProjectionMatrix());
				// Bind pipeline
				cmd.BindPipeline(shadowPipeline[0]);
				// Bind data descriptor sets
				cmd.BindDescriptorSets(shadowPipeline, { shadowPipeline.descriptorSets[0][this->renderer.frameIndex].set }, { 0 });
				// Bind storage descriptor sets
				cmd.BindDescriptorSet(shadowPipeline, this->renderer.ssbo[this->renderer.frameIndex].set, 0, 1);
				for (uint32_t i = 0; i < this->renderer.meshes.capacity() - 2; i++)
				{
					const Crescendo::Vulkan::Mesh& mesh = this->renderer.meshes[i];
					// If outside the frustum, skip
					if (!frustum.intersects(this->modelData[i].bounds)) continue;
					// Bind vertex meshes
					std::vector<VkBuffer> buffers = shadowPipeline.GetMatchingBuffers(mesh);
					const std::vector<VkDeviceSize> bufferOffsets(buffers.size(), 0);
					cmd.BindVertexBuffers(buffers, bufferOffsets);
					cmd.BindIndexBuffer(mesh.indexBuffer);
					cmd.DrawIndexed(mesh.indexCount, 1, 0, 0, i);
				}
			}
			cmd.EndRenderPass();
			// Normal pass
			cmd.DynamicStateSetViewport(offScreen.GetViewport(true));
			cmd.DynamicStateSetScissor(offScreen.GetScissor());
			cmd.BeginRenderPass(offScreen.renderPass, offScreen, offScreen.GetScissor(), { { 0.0f, 0.0f, 0.0f, 1.0f }, Crescendo::Vulkan::Create::DefaultDepthClear() });
			{
				// Normal rendering
				cs_std::graphics::frustum frustum = cs_std::graphics::frustum(this->camera.camera.GetViewProjectionMatrix());
				for (uint32_t i = 0; i < this->renderer.meshes.capacity() - 2; i++)
				{
					const Crescendo::Vulkan::Mesh& mesh = this->renderer.meshes[i];
					// If outside the frustum, skip
					if (!frustum.intersects(this->modelData[i].bounds)) continue;
					// Select the correct pipeline
					int index = (this->modelData[i].isDoubleSided << 1) | this->modelData[i].isTransparent;
					cmd.BindPipeline(defaultPipeline[index]);
					// Bind data descriptor sets
					cmd.BindDescriptorSets(defaultPipeline, { defaultPipeline.descriptorSets[0][this->renderer.frameIndex].set, defaultPipeline.descriptorSets[1][0].set }, { 0, 0, 0 });
					// Bind storage descriptor sets
					cmd.BindDescriptorSet(defaultPipeline, this->renderer.ssbo[this->renderer.frameIndex].set, 0, 2);
					// Bind texture descriptor sets
					cmd.BindDescriptorSets(defaultPipeline, { this->renderer.textures[this->modelData[i].textureID].set, this->renderer.textures[this->modelData[i].normalID].set, this->renderer.textures[this->renderer.shadowMap.textureIndex].set }, {}, 3);
					// Bind vertex meshes
					std::vector<VkBuffer> buffers = defaultPipeline.GetMatchingBuffers(mesh);
					const std::vector<VkDeviceSize> bufferOffsets(buffers.size(), 0);
					cmd.BindVertexBuffers(buffers, bufferOffsets);
					cmd.BindIndexBuffer(mesh.indexBuffer);
					cmd.DrawIndexed(mesh.indexCount, 1, 0, 0, i);
				}
				// Skybox rendering
				{
					cmd.BindPipeline(skyboxPipeline[0]);
					// Bind data descriptor sets
					cmd.BindDescriptorSets(skyboxPipeline, { skyboxPipeline.descriptorSets[0][this->renderer.frameIndex].set }, { 0 });
					// Bind texture descriptor sets
					cmd.BindDescriptorSet(skyboxPipeline, this->renderer.textures[this->modelData[this->renderer.meshes.capacity() - 2].textureID].set, 0, 1);
					// Bind vertex meshes
					std::vector<VkBuffer> buffers = skyboxPipeline.GetMatchingBuffers(this->renderer.meshes[this->renderer.meshes.capacity() - 2]);
					const std::vector<VkDeviceSize> bufferOffsets(buffers.size(), 0);
					cmd.BindVertexBuffers(buffers, bufferOffsets);
					cmd.BindIndexBuffer(this->renderer.meshes[this->renderer.meshes.capacity() - 2].indexBuffer);
					cmd.DrawIndexed(this->renderer.meshes[this->renderer.meshes.capacity() - 2].indexCount, 1, 0, 0, this->renderer.meshes.capacity() - 2);
				}
				// UI rendering
				{
					cmd.BindPipeline(uiPipeline[0]);
					// Bind data descriptor sets
					cmd.BindDescriptorSets(uiPipeline, { uiPipeline.descriptorSets[0][this->renderer.frameIndex].set }, { 0 });
					// Bind storage descriptor sets
					cmd.BindDescriptorSet(uiPipeline, this->renderer.ssbo[this->renderer.frameIndex].set, 0, 1);
					// Bind texture descriptor sets
					cmd.BindDescriptorSet(uiPipeline, this->renderer.textures[this->renderer.shadowMap.textureIndex].set, 0, 2);
					// Bind vertex meshes
					std::vector<VkBuffer> buffers = uiPipeline.GetMatchingBuffers(this->renderer.meshes[this->renderer.meshes.capacity() - 1]);
					const std::vector<VkDeviceSize> bufferOffsets(buffers.size(), 0);
					cmd.BindVertexBuffers(buffers, bufferOffsets);
					cmd.BindIndexBuffer(this->renderer.meshes[this->renderer.meshes.capacity() - 1].indexBuffer);
					cmd.DrawIndexed(this->renderer.meshes[this->renderer.meshes.capacity() - 1].indexCount, 1, 0, 0, this->renderer.meshes.capacity() - 1);
				}
			}
			cmd.EndRenderPass();
			// Post-processing step
			cmd.DynamicStateSetViewport(framebuffer.GetViewport(false));
			cmd.DynamicStateSetScissor(framebuffer.GetScissor());
			cmd.BeginRenderPass(framebuffer.renderPass, framebuffer, framebuffer.GetScissor(), { { 0.0f, 0.0f, 0.0f, 1.0f } });
			{
				cmd.BindPipeline(postProcessingPipeline[0]);
				// Bind texture
				cmd.BindDescriptorSet(postProcessingPipeline, this->renderer.textures[this->renderer.offscreen.textureIndex].set, 0, 0);
				cmd.Draw(6);
			}
			cmd.EndRenderPass();
		cmd.End();
		cmd.Submit(cur.presentReady, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, cur.renderFinish);

		/* ---------------------------------------------------------------- Post-render and present ---------------------------------------------------------------- */

		cmd.Present(this->renderer.swapchain, currentImage, cur.renderFinish);
		this->renderer.NextFrame();
		
		// Frame counter
		frame++;
		if (this->GetTime() - this->lastTime >= 1.0)
		{
			this->lastTime = this->GetTime();
			this->GetWindow()->SetName("Crescendo | FPS: " + std::to_string(this->frame));
			this->frame = 0;
		}

		if (Input::GetKeyDown(Key::F11)) this->GetWindow()->SetFullScreen(!this->GetWindow()->IsFullScreen());
		if (Input::GetMouseButtonDown(MouseButton::Left)) this->GetWindow()->SetCursorLock(true);
		if (Input::GetKeyDown(Key::Escape))
		{
			if (this->GetWindow()->IsCursorLocked()) this->GetWindow()->SetCursorLock(false);
			else this->Exit();
		}
		if (Input::GetKeyPressed(Key::ControlLeft) && Input::GetKeyPressed(Key::F5)) this->Restart();
	}
	void OnExit()
	{
		std::string xml = CVar::SerializeConfigXML();
		cs_std::console::log(xml);
	}
};

CrescendoRegisterApp(Sandbox);
#define CS_SHOW_TIMINGS
#include "Crescendo.hpp"
using namespace CrescendoEngine;

#include "cs_std/math/math.hpp"
#include "cs_std/graphics/algorithms.hpp"
#include "cs_std/xml/xml.hpp"
namespace math = cs_std::math;

#include "CameraController.hpp"

class Sandbox : public Application
{
private:
	CameraController camera;
	OrthographicCamera UICamera;
	OrthographicCamera shadowMapCamera;

	EntityManager entityManager;

	cs_std::packed_vector<Entity> entities;

	uint32_t skyboxEntityIdx, activeCameraIdx;

	int frame = 0;
	double lastTime = 0.0;

public:
	void LoadModels(std::vector<cs_std::graphics::model>& models)
	{
		uint32_t textureIndex = 0;
		const uint32_t currentTextureCount = this->renderer.textures.capacity();
		std::map<std::filesystem::path, uint32_t> seenTextures;

		uint32_t indexCount = 0;

		for (auto& model : models)
		{
			for (uint32_t i = 0; i < model.meshes.size(); i++)
			{
				auto& mesh = model.meshes[i];
				auto& attributes = model.meshAttributes[i];

				if (!mesh.has_attribute(cs_std::graphics::Attribute::TANGENT)) cs_std::graphics::generate_tangents(mesh);
				uint32_t meshID = this->renderer.meshes.insert(this->renderer.UploadMesh(mesh));
				if (!attributes.diffuse.empty() && seenTextures.find(attributes.diffuse) == seenTextures.end()) { seenTextures[attributes.diffuse] = textureIndex; textureIndex++; }
				if (!attributes.normal.empty() && seenTextures.find(attributes.normal) == seenTextures.end()) { seenTextures[attributes.normal] = textureIndex; textureIndex++; }

				for (auto& ssbo : this->renderer.ssbo) memcpy(static_cast<char*>(ssbo.buffer.mPtr) + sizeof(math::mat4) * meshID, &attributes.transform, sizeof(math::mat4));

				Entity entity = entityManager.CreateEntity();
				entity.EmplaceComponent<Name>("Mesh");
				entity.EmplaceComponent<Transform>(attributes.transform);
				entity.EmplaceComponent<MeshData>(cs_std::graphics::bounding_aabb(mesh.get_attribute(cs_std::graphics::Attribute::POSITION).data).transform(attributes.transform), meshID);
				entity.EmplaceComponent<Material>(
					0, seenTextures[attributes.diffuse] + currentTextureCount, seenTextures[attributes.normal] + currentTextureCount,
					attributes.isTransparent, attributes.isDoubleSided, true
				);
				entities.insert(entity);
			}
		}

		/* ---------------------------------------------------------------- 1.3 - Texture Data ---------------------------------------------------------------- */

		seenTextures.erase("");
		std::vector<std::filesystem::path> textureStrings(seenTextures.size());
		for (const auto& texture : seenTextures) textureStrings[texture.second] = texture.first;

		std::vector<cs_std::image> images(textureStrings.size());

		this->taskQueue.wake();
		uint32_t last = 0;
		for (uint32_t i = 0; i < textureStrings.size(); i++)
		{
			this->taskQueue.push_back([&images, &textureStrings, i]() { images[i] = LoadImage(textureStrings[i]); });
		}

		while (!this->taskQueue.finished())
		{
			uint32_t local = textureStrings.size() - this->taskQueue.pending_task_count();
			for (uint32_t i = 0; i < local - last; i++) cs_std::console::raw("#");
			last = local;
		}
		cs_std::console::raw('\n');
		this->taskQueue.sleep();

		for (auto& image : images) this->renderer.textures.insert(this->renderer.UploadTexture(image, true));

		lastTime = this->GetTime();
	}
	void OnStartup()
	{

		this->GetWindow()->SetCursorLock(true);

		this->camera = CameraController(70.0f, this->GetWindow()->GetAspectRatio(), { 0.1f, 1000.0f });
		this->camera.camera.SetPosition(math::vec3(0.0f, 0.0f, 0.0f));
		this->UICamera = OrthographicCamera(math::vec4(0.0f, this->GetWindow()->GetWidth(), this->GetWindow()->GetHeight(), 0.0f), math::vec2(-1.0f, 1.0f));
		this->UICamera.SetPosition(math::vec3(0.0f, 0.0f, 0.0f));
		this->UICamera.SetRotation(math::quat(0.0f, 1.0f, 0.0f, 0.0f));

		this->shadowMapCamera = OrthographicCamera(math::vec4(-12.5f, 12.5f, -20.0f, 20.0f), math::vec2(0.0f, 100.0f));

		/* ---------------------------------------------------------------- 1.0 - Shader data ---------------------------------------------------------------- */
		struct Shader { std::string name; Vulkan::PipelineVariants variants; };
		std::string shaderPath = CVar::Get<std::string>("pc_shaderpath");

		std::vector<Shader> shaderList{
			{ "default", Vulkan::PipelineVariants::GetDefaultVariant(this->renderer.renderPasses[0], this->renderer.specs.multisamples) },
			{ "skybox", Vulkan::PipelineVariants::GetSkyboxVariant(this->renderer.renderPasses[0], this->renderer.specs.multisamples) },
			{ "depth", Vulkan::PipelineVariants::GetShadowVariant(this->renderer.renderPasses[2]) }, // Shadowmap
			{ "ui", Vulkan::PipelineVariants::GetUIVariant(this->renderer.renderPasses[0], this->renderer.specs.multisamples) },
			{ "post_processing", Vulkan::PipelineVariants::GetPostProcessingVariant(this->renderer.renderPasses[1]) },
			{ "depth", Vulkan::PipelineVariants::GetDepthPrepassVariant(this->renderer.renderPasses[3], this->renderer.specs.multisamples) } // Depth pre-pass
		};

		for (const auto& shader : shaderList)
		{
			this->renderer.pipelines.insert(this->renderer.device.CreatePipelines(
				cs_std::binary_file(shaderPath + shader.name + ".vert.spv").open().read_if_exists(),
				cs_std::binary_file(shaderPath + shader.name + ".frag.spv").open().read_if_exists(),
				shader.variants
			));
		}

		/* ---------------------------------------------------------------- 1.1 - Descriptor Data ---------------------------------------------------------------- */

		this->renderer.pipelines[0].CreateDescriptorSets(0, 3); // One per frame in flight (fif)
		this->renderer.pipelines[0].CreateDescriptorSet(1);
		this->renderer.pipelines[1].CreateDescriptorSets(0, 3); // One per fif
		this->renderer.pipelines[2].CreateDescriptorSets(0, 3); // One per fif
		this->renderer.pipelines[3].CreateDescriptorSets(0, 3); // One per fif
		this->renderer.pipelines[5].CreateDescriptorSets(0, 3); // One per fif

		this->renderer.pipelines[0].UpdateDescriptorData(0, 1, 0, math::vec3(0.3f, 0.4f, 1.0f));

		/* ---------------------------------------------------------------- 1.2 - Mesh data ---------------------------------------------------------------- */

		{
			cs_std::graphics::model skyboxModel {};
			Construct::Mesh skybox = Construct::SkyboxSphere(16, 16);
			cs_std::graphics::mesh skyboxMesh {};
			skyboxMesh.indices = skybox.indices;
			skyboxMesh.add_attribute(cs_std::graphics::Attribute::POSITION, skybox.vertices)
				.add_attribute(cs_std::graphics::Attribute::TEXCOORD_0, skybox.textureUVs);
			cs_std::graphics::mesh_attributes skyboxAttributes = cs_std::graphics::mesh_attributes().set_diffuse("./assets/skybox.png");
			skyboxModel.add_mesh(skyboxMesh, skyboxAttributes);

			uint32_t meshID = this->renderer.meshes.insert(this->renderer.UploadMesh(skyboxMesh));
			uint32_t textureID = this->renderer.textures.insert(this->renderer.UploadTexture(LoadImage("./assets/skybox.png"), true));

			Entity skyboxEntity = entityManager.CreateEntity();
			skyboxEntity.EmplaceComponent<Name>("Skybox");
			skyboxEntity.EmplaceComponent<Skybox>(meshID, textureID);
			skyboxEntityIdx = entities.insert(skyboxEntity);
		}

		std::string assetPath = CVar::Get<std::string>("pc_assetpath");

		cs_std::xml::document modelsXML(cs_std::text_file("./models.xml").open().read());
		std::vector<cs_std::graphics::model> models;

		for (const auto& model : modelsXML)
		{
			if (model->tag == "gltf") models.push_back(LoadGLTF(assetPath + model->innerText));
			else if (model->tag == "obj") models.push_back(LoadOBJ(assetPath + model->innerText));
		}
		LoadModels(models);
	}
	void OnUpdate(double dt)
	{
		/* ---------------------------------------------------------------- Game update ---------------------------------------------------------------- */

		this->camera.OnUpdate();

		float currentTime = this->GetTime() / 10.0f;
		this->shadowMapCamera.SetPosition(math::vec3(std::sinf(currentTime) * 75.0f, std::cosf(currentTime) * 75.0f, 0.0f));
		this->shadowMapCamera.LookAt(math::vec3(0.0f, 0.0f, 0.0f));

		/* ---------------------------------------------------------------- Render preparation ---------------------------------------------------------------- */

		Vulkan::Pipelines& defaultPipeline = this->renderer.pipelines[0];
		Vulkan::Pipelines& skyboxPipeline = this->renderer.pipelines[1];
		Vulkan::Pipelines& shadowPipeline = this->renderer.pipelines[2];
		Vulkan::Pipelines& uiPipeline = this->renderer.pipelines[3];
		Vulkan::Pipelines& postProcessingPipeline = this->renderer.pipelines[4];
		Vulkan::Pipelines& depthPrepassPipeline = this->renderer.pipelines[5];

		// Render prep
		const math::mat4 projections[2]{ this->camera.camera.GetViewProjectionMatrix(), this->shadowMapCamera.GetViewProjectionMatrix() };
		const math::vec4 lightingPositions[2]{ math::vec4(this->shadowMapCamera.GetPosition(), 1.0f), math::vec4(this->camera.camera.GetPosition(), 1.0f) };

		// Arguments in order: set index, set, binding, data
		defaultPipeline.UpdateDescriptorData(this->renderer.frameIndex, 0, 0, projections);
		defaultPipeline.UpdateDescriptorData(this->renderer.frameIndex, 0, 1, lightingPositions);
		skyboxPipeline.UpdateDescriptorData(this->renderer.frameIndex, 0, 0, math::translate(this->camera.camera.GetViewProjectionMatrix(), this->camera.camera.GetPosition()));
		shadowPipeline.UpdateDescriptorData(this->renderer.frameIndex, 0, 0, projections[1]);
		uiPipeline.UpdateDescriptorData(this->renderer.frameIndex, 0, 0, this->UICamera.GetViewProjectionMatrix());
		depthPrepassPipeline.UpdateDescriptorData(this->renderer.frameIndex, 0, 0, projections[0]);

		/* ---------------------------------------------------------------- Render commands ---------------------------------------------------------------- */

		Vulkan::RenderCommandQueue& cur = this->renderer.GetCurrentRenderCommandQueue();
		Vulkan::GraphicsCommandQueue& cmd = cur.cmd;

		cmd.WaitCompletion();
		cmd.Reset();

		// Get the next image
		uint32_t currentImage = this->renderer.swapchain.AcquireNextImage(cur.presentReady);
		if (this->renderer.swapchain.NeedsRecreation())
		{
			this->renderer.CreateSwapchain();
			currentImage = this->renderer.swapchain.AcquireNextImage(cur.presentReady);
		}

		const Vulkan::Framebuffer& postProcessing = this->renderer.framebuffers[currentImage];
		const Vulkan::Framebuffer& offScreen = this->renderer.framebuffers[this->renderer.offscreen.framebufferIndex];
		const Vulkan::Framebuffer& shadow = this->renderer.framebuffers[this->renderer.shadowMap.framebufferIndex];
		const Vulkan::Framebuffer& depthPrepass = this->renderer.framebuffers[this->renderer.depthPrepass.framebufferIndex];

		cs_std::graphics::frustum shadowMapFrustum(this->shadowMapCamera.GetViewProjectionMatrix()), cameraFrustum(this->camera.camera.GetViewProjectionMatrix());

		// Meshes to render
		struct RenderData { MeshData* mesh; Material* material; };
		std::vector<RenderData> shadowMapRenderData = {}, solidRenderData = {}, transparentRenderData = {};


		//for (auto& entity : entities)
		for (size_t i = 0; i < entities.size(); i++)
		{
			if (!entities.is_valid(i)) continue;
			Entity& entity = entities[i];
			if (entity.HasComponents<MeshData, Material>())
			{
				auto& meshData = entity.GetComponent<MeshData>();
				auto& meshMaterial = entity.GetComponent<Material>();

				// Shadow map culling
				if (meshMaterial.isShadowCasting && shadowMapFrustum.intersects(meshData.bounds))
				{
					shadowMapRenderData.emplace_back(&meshData, &meshMaterial);
				}
				// Camera culling
				if (cameraFrustum.intersects(meshData.bounds))
				{
					if (meshMaterial.isTransparent) transparentRenderData.emplace_back(&meshData, &meshMaterial);
					else solidRenderData.emplace_back(&meshData, &meshMaterial);
				}
			}
		}

		cmd.Begin();
		// Shadowmap pass
		{
			const VkRect2D scissor = shadow.GetScissor();
			cmd.DynamicStateSetViewport(shadow.GetViewport());
			cmd.DynamicStateSetScissor(scissor);
			cmd.BeginRenderPass(shadow.renderPass, shadow, scissor, { Vulkan::Create::DefaultDepthClear() });
			cmd.BindPipeline(shadowPipeline[0]);
			cmd.BindDescriptorSets(shadowPipeline, { shadowPipeline.descriptorSets[0][this->renderer.frameIndex].set }, { 0 });
			cmd.BindDescriptorSet(shadowPipeline, this->renderer.ssbo[this->renderer.frameIndex].set, 0, 1);
			for (auto& renderData : shadowMapRenderData)
			{
				const Vulkan::Mesh& mesh = this->renderer.meshes[renderData.mesh->meshID];
				std::vector<VkBuffer> buffers = shadowPipeline.GetMatchingBuffers(mesh);
				const std::vector<VkDeviceSize> bufferOffsets(buffers.size(), 0);
				cmd.BindVertexBuffers(buffers, bufferOffsets);
				cmd.BindIndexBuffer(mesh.indexBuffer);
				cmd.DrawIndexed(mesh.indexCount, 1, 0, 0, renderData.mesh->meshID);
			}
			cmd.EndRenderPass();
		}
		// Depth pre-pass
		{
			const VkRect2D scissor = depthPrepass.GetScissor();
			cmd.DynamicStateSetViewport(depthPrepass.GetViewport(true));
			cmd.DynamicStateSetScissor(scissor);
			cmd.BeginRenderPass(depthPrepass.renderPass, depthPrepass, scissor, { Vulkan::Create::DefaultDepthClear() });
			cmd.BindPipeline(depthPrepassPipeline[0]);
			// Normal rendering
			{
				for (auto& renderData : solidRenderData)
				{
					const Vulkan::Mesh& mesh = this->renderer.meshes[renderData.mesh->meshID];
					cmd.BindDescriptorSets(depthPrepassPipeline, { depthPrepassPipeline.descriptorSets[0][this->renderer.frameIndex].set }, { 0 });
					cmd.BindDescriptorSet(depthPrepassPipeline, this->renderer.ssbo[this->renderer.frameIndex].set, 0, 1);
					std::vector<VkBuffer> buffers = depthPrepassPipeline.GetMatchingBuffers(mesh);
					const std::vector<VkDeviceSize> bufferOffsets(buffers.size(), 0);
					cmd.BindVertexBuffers(buffers, bufferOffsets);
					cmd.BindIndexBuffer(mesh.indexBuffer);
					cmd.DrawIndexed(mesh.indexCount, 1, 0, 0, renderData.mesh->meshID);
				}
			}
			cmd.EndRenderPass();
		}
		// Normal pass
		{
			const VkRect2D scissor = offScreen.GetScissor();
			cmd.DynamicStateSetViewport(offScreen.GetViewport(true));
			cmd.DynamicStateSetScissor(scissor);
			cmd.BeginRenderPass(offScreen.renderPass, offScreen, scissor, { { 0.0f, 0.0f, 0.0f, 1.0f } });
			// Skybox rendering
			{
				Skybox& skyboxData = entities[skyboxEntityIdx].GetComponent<Skybox>();

				cmd.BindPipeline(skyboxPipeline[0]);
				cmd.BindDescriptorSets(skyboxPipeline, { skyboxPipeline.descriptorSets[0][this->renderer.frameIndex].set }, { 0 });
				cmd.BindDescriptorSet(skyboxPipeline, this->renderer.textures[skyboxData.textureID].set, 0, 1);
				std::vector<VkBuffer> buffers = skyboxPipeline.GetMatchingBuffers(this->renderer.meshes[skyboxData.meshID]);
				cmd.BindVertexBuffers(buffers, std::vector<VkDeviceSize>(buffers.size(), 0));
				cmd.BindIndexBuffer(this->renderer.meshes[skyboxData.meshID].indexBuffer);
				cmd.DrawIndexed(this->renderer.meshes[skyboxData.meshID].indexCount, 1, 0, 0, 0);
			}
			// Solid objects rendering
			{
				for (auto& renderData : solidRenderData)
				{
					const Vulkan::Mesh& mesh = this->renderer.meshes[renderData.mesh->meshID];
					cmd.BindPipeline(defaultPipeline[renderData.material->isDoubleSided]);
					cmd.BindDescriptorSets(defaultPipeline, { defaultPipeline.descriptorSets[0][this->renderer.frameIndex].set, defaultPipeline.descriptorSets[1][0].set }, { 0, 0, 0 });
					cmd.BindDescriptorSet(defaultPipeline, this->renderer.ssbo[this->renderer.frameIndex].set, 0, 2);
					cmd.BindDescriptorSets(defaultPipeline, { this->renderer.textures[renderData.material->diffuseID].set, this->renderer.textures[renderData.material->normalID].set, this->renderer.textures[this->renderer.shadowMap.textureIndices[0]].set }, {}, 3);
					std::vector<VkBuffer> buffers = defaultPipeline.GetMatchingBuffers(mesh);
					const std::vector<VkDeviceSize> bufferOffsets(buffers.size(), 0);
					cmd.BindVertexBuffers(buffers, bufferOffsets);
					cmd.BindIndexBuffer(mesh.indexBuffer);
					cmd.DrawIndexed(mesh.indexCount, 1, 0, 0, renderData.mesh->meshID);
				}
			}
			// Transparent objects rendering
			{
				for (auto& renderData : transparentRenderData)
				{
					const Vulkan::Mesh& mesh = this->renderer.meshes[renderData.mesh->meshID];
					cmd.BindPipeline(defaultPipeline[renderData.material->isDoubleSided]);
					cmd.BindDescriptorSets(defaultPipeline, { defaultPipeline.descriptorSets[0][this->renderer.frameIndex].set, defaultPipeline.descriptorSets[1][0].set }, { 0, 0, 0 });
					cmd.BindDescriptorSet(defaultPipeline, this->renderer.ssbo[this->renderer.frameIndex].set, 0, 2);
					cmd.BindDescriptorSets(defaultPipeline, { this->renderer.textures[renderData.material->diffuseID].set, this->renderer.textures[renderData.material->normalID].set, this->renderer.textures[this->renderer.shadowMap.textureIndices[0]].set }, {}, 3);
					std::vector<VkBuffer> buffers = defaultPipeline.GetMatchingBuffers(mesh);
					const std::vector<VkDeviceSize> bufferOffsets(buffers.size(), 0);
					cmd.BindVertexBuffers(buffers, bufferOffsets);
					cmd.BindIndexBuffer(mesh.indexBuffer);
					cmd.DrawIndexed(mesh.indexCount, 1, 0, 0, renderData.mesh->meshID);
				}
			}
			cmd.EndRenderPass();
		}
		// Post-processing step
		{
			const VkRect2D scissor = postProcessing.GetScissor();
			cmd.DynamicStateSetViewport(postProcessing.GetViewport());
			cmd.DynamicStateSetScissor(scissor);
			cmd.BeginRenderPass(postProcessing.renderPass, postProcessing, scissor, { { 0.0f, 0.0f, 0.0f, 1.0f } });
			cmd.BindPipeline(postProcessingPipeline[0]);
			cmd.BindDescriptorSet(postProcessingPipeline, this->renderer.textures[this->renderer.offscreen.textureIndices[0]].set, 0, 0);
			cmd.Draw(6);
			cmd.EndRenderPass();
		}
		cmd.End();
		cmd.Submit(cur.presentReady, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, cur.renderFinish);

		/* ---------------------------------------------------------------- Post-render and present ---------------------------------------------------------------- */

		cmd.Present(this->renderer.swapchain, currentImage, cur.renderFinish);
		this->renderer.NextFrame();

		// Frame counter
		frame++;
		if (this->GetTime() - this->lastTime >= 1.0)
		{
			this->lastTime += 1.0;
			this->GetWindow()->SetName("Crescendo | FPS: " + std::to_string(this->frame));
			this->frame = 0;
		}

		if (Input::GetKeyDown(Key::F11)) this->GetWindow()->SetFullScreen(!this->GetWindow()->IsFullScreen());
		if (Input::GetMouseButtonDown(MouseButton::Left)) this->GetWindow()->SetCursorLock(true);
		if (Input::GetKeyDown(Key::Escape)) this->GetWindow()->IsCursorLocked() ? this->GetWindow()->SetCursorLock(false) : this->Exit();
		if (Input::GetKeyPressed(Key::ControlLeft) && Input::GetKeyPressed(Key::F5)) this->Restart();
	}
	void OnExit()
	{
		cs_std::console::log("Exiting...");
	}
};

CrescendoRegisterApp(Sandbox);
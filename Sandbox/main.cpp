#define CS_SHOW_TIMINGS
#include "Crescendo.hpp"
using namespace CrescendoEngine;

#include "cs_std/math/math.hpp"
#include "cs_std/graphics/algorithms.hpp"
#include "cs_std/xml/xml.hpp"
namespace math = cs_std::math;

#include "scripts/CameraController.hpp"

class Sandbox : public Application
{
private:
	cs_std::packed_vector<Entity> entities;
	uint32_t skyboxEntityIdx, activeCameraIdx;
	std::vector<uint32_t> transformSSBOIdx;
	std::vector<uint32_t> directionalLightSSBOIdx, pointLightSSBOIdx, spotLightSSBOIdx;

	int frame = 0;
	double lastTime = 0.0;
public:
	void LoadModels(std::vector<cs_std::graphics::model>& models)
	{
		uint32_t textureIndex = 0;
		const uint32_t currentTextureCount = this->renderer.textures.capacity();

		struct TextureInfo { uint32_t textureIdx; Colorspace colorspace; };
		std::map<std::filesystem::path, TextureInfo> seenTextures;

		uint32_t indexCount = 0;

		for (auto& model : models)
		{
			for (uint32_t i = 0; i < model.meshes.size(); i++)
			{
				auto& mesh = model.meshes[i];
				auto& attributes = model.meshAttributes[i];

				if (!mesh.has_attribute(cs_std::graphics::Attribute::TANGENT)) cs_std::graphics::generate_tangents(mesh);
				uint32_t meshID = this->renderer.UploadMesh(mesh);
				if (!attributes.diffuse.empty() && seenTextures.find(attributes.diffuse) == seenTextures.end())
				{
					seenTextures[attributes.diffuse].textureIdx = textureIndex;
					seenTextures[attributes.diffuse].colorspace = Colorspace::SRGB;
					textureIndex++;
				}
				if (!attributes.normal.empty() && seenTextures.find(attributes.normal) == seenTextures.end())
				{
					seenTextures[attributes.normal].textureIdx = textureIndex;
					seenTextures[attributes.normal].colorspace = Colorspace::Linear;
					textureIndex++;
				}

				for (auto& SSBOIdx : this->transformSSBOIdx) memcpy(static_cast<char*>(this->renderer.SSBOs[SSBOIdx].buffer.mPtr) + sizeof(math::mat4) * meshID, &attributes.transform, sizeof(math::mat4));

				Entity entity = entityManager.CreateEntity();
				entity.EmplaceComponent<Name>("Mesh");
				entity.EmplaceComponent<Transform>(attributes.transform);
				entity.EmplaceComponent<MeshData>(cs_std::graphics::bounding_aabb(mesh.get_attribute(cs_std::graphics::Attribute::POSITION).data).transform(attributes.transform), meshID);
				entity.EmplaceComponent<Material>(
					0, seenTextures[attributes.diffuse].textureIdx + currentTextureCount, seenTextures[attributes.normal].textureIdx + currentTextureCount,
					attributes.isTransparent, attributes.isDoubleSided, true
				);
				entities.insert(entity);
			}
		}

		/* ---------------------------------------------------------------- 1.3 - Texture Data ---------------------------------------------------------------- */

		seenTextures.erase("");
		std::vector<std::filesystem::path> textureStrings(seenTextures.size());
		for (const auto& texture : seenTextures) textureStrings[texture.second.textureIdx] = texture.first;

		struct TaggedImage
		{
			cs_std::image image;
			Colorspace colorspace;
		};

		std::vector<cs_std::image> images(textureStrings.size());
		for (uint32_t i = 0; i < textureStrings.size(); i++)
		{
			this->taskQueue.push_back([&images, &textureStrings, i]() { images[i] = LoadImage(textureStrings[i]); });
		}

		uint32_t last = 0;
		while (!this->taskQueue.finished())
		{
			uint32_t local = textureStrings.size() - this->taskQueue.pending_task_count();
			for (uint32_t i = 0; i < local - last; i++) cs_std::console::raw("#");
			last = local;
		}
		cs_std::console::raw('\n');
		this->taskQueue.sleep();

		for (uint32_t i = 0; i < images.size(); i++)
		{
			auto& image = images[i];
			this->renderer.UploadTexture(image, seenTextures[textureStrings[i]].colorspace, true);
		}

		lastTime = this->GetTime();
	}
	void OnStartup()
	{
		this->GetWindow()->SetCursorLock(true);
		//this->UICamera = OrthographicCamera(math::vec4(0.0f, this->GetWindow()->GetWidth(), this->GetWindow()->GetHeight(), 0.0f), math::vec2(-1.0f, 1.0f));
		//this->UICamera.SetPosition(math::vec3(0.0f, 0.0f, 0.0f));
		//this->UICamera.SetRotation(math::quat(0.0f, 1.0f, 0.0f, 0.0f));

		// Create Camera
		{
			Entity cameraEntity = entityManager.CreateEntity();
			cameraEntity.EmplaceComponent<Name>("Main Camera");
			cameraEntity.EmplaceComponent<Transform>(math::vec3(0.0f, 0.0f, 0.0f));
			cameraEntity.EmplaceComponent<PerspectiveCamera>(70.0f, this->GetWindow()->GetAspectRatio(), 0.1f, 1000.0f);
			cameraEntity.EmplaceComponent<Behaviours>(std::make_shared<CameraController>());
			cameraEntity.EmplaceComponent<SpotLight>(glm::vec3(1.0f, 1.0f, 1.0f), 25.0f, math::radians(1.0f), math::radians(30.0f), true);
			activeCameraIdx = entities.insert(cameraEntity);
		}

		// Create shadow casting directional light
		{
			Entity skyLight1 = entityManager.CreateEntity();
			skyLight1.EmplaceComponent<Name>("Default Sunlight");
			skyLight1.EmplaceComponent<Transform>(math::vec3(0.0f, 75.0f, 45.0f)).LookAt(math::vec3(0.0f, 0.0f, 0.0f));
			// skyLight1.EmplaceComponent<OrthographicCamera>(-12.5f, 12.5f, -20.0f, 20.0f, 0.0f, 100.0f);
			skyLight1.EmplaceComponent<DirectionalLight>(glm::vec3(0.992f, 0.984f, 0.827f), 0.25f, true);
			entities.insert(skyLight1);

			std::vector<math::vec3> pointLights =
			{
				math::vec3(15.5f, 3.25f, -0.9f),
				math::vec3(-13.5f, 4.0f, 0.0f),

				math::vec3(10.25f, 4.0f, 4.75f),
				math::vec3(6.25f, 4.0f, 4.75f),
				math::vec3(2.25f, 4.0f, 4.75f),
				math::vec3(-1.75f, 4.0f, 4.75f),
				math::vec3(-5.75f, 4.0f, 4.75f),
				math::vec3(-9.75f, 4.0f, 4.75f),

				math::vec3(10.25f, 4.0f, -4.75f),
				math::vec3(6.25f, 4.0f, -4.75f),
				math::vec3(2.25f, 4.0f, -4.75f),
				math::vec3(-1.75f, 4.0f, -4.75f),
				math::vec3(-5.75f, 4.0f, -4.75f),
				math::vec3(-9.75f, 4.0f, -4.75f),

				math::vec3(10.2f, 9.25f, 4.75f),
				math::vec3(1.8f, 9.25f, 4.75f),
				math::vec3(-6.0f, 9.25f, 4.75f),
				math::vec3(-9.6f, 9.25f, 4.75f),

				math::vec3(10.2f, 9.25f, -4.75f),
				math::vec3(1.8f, 9.25f, -4.75f),
				math::vec3(-6.0f, 9.25f, -4.75f),
				math::vec3(-9.6f, 9.25f, -4.75f)
			};

			for (auto& light : pointLights)
			{
				Entity pointLight = entityManager.CreateEntity();
				pointLight.EmplaceComponent<Name>("Default Pointlight");
				pointLight.EmplaceComponent<Transform>(light);
				pointLight.EmplaceComponent<PointLight>(glm::vec3(1.0f, 0.654f, 0.341f), 2.5f, true);
				entities.insert(pointLight);
			}

			Entity spotLight = entityManager.CreateEntity();
			spotLight.EmplaceComponent<Name>("Default Spotlight");
			spotLight.EmplaceComponent<Transform>(math::vec3(15.0f, 1.0f, 0.0f));
			spotLight.GetComponent<Transform>().LookAt(math::vec3(0.0f, 1.0f, 0.0f));
			spotLight.EmplaceComponent<SpotLight>(glm::vec3(1.0f, 1.0f, 1.0f), 500.0f, math::radians(1.0f), math::radians(12.5f), true);
			entities.insert(spotLight);

		}

		for (uint32_t i = 0; i < this->renderer.specs.framesInFlight; i++)
		{
			this->transformSSBOIdx.push_back(this->renderer.CreateSSBO(sizeof(Transform) * CVar::Get<uint64_t>("irc_maxobjectcount"), VK_SHADER_STAGE_VERTEX_BIT));
			this->directionalLightSSBOIdx.push_back(this->renderer.CreateSSBO(sizeof(DirectionalLight::ShaderRepresentation) * CVar::Get<uint64_t>("irc_maxdirectionallightcount"), VK_SHADER_STAGE_FRAGMENT_BIT));
			this->pointLightSSBOIdx.push_back(this->renderer.CreateSSBO(sizeof(PointLight::ShaderRepresentation) * CVar::Get<uint64_t>("irc_maxpointlightcount"), VK_SHADER_STAGE_FRAGMENT_BIT));
			this->spotLightSSBOIdx.push_back(this->renderer.CreateSSBO(sizeof(SpotLight::ShaderRepresentation) * CVar::Get<uint64_t>("irc_maxspotlightcount"), VK_SHADER_STAGE_FRAGMENT_BIT));
		}

		// this->shadowMapIdx = this->renderer.CreateShadowMap(this->renderer.renderPasses[2], VK_FORMAT_D16_UNORM, 4096, 4096);

		/* ---------------------------------------------------------------- 1.0 - Shader data ---------------------------------------------------------------- */
		struct Shader { std::string name; Vulkan::PipelineVariants variants; };
		std::string shaderPath = CVar::Get<std::string>("pc_shaderpath");

		std::vector<Shader> shaderList {
			{ "default", Vulkan::PipelineVariants::GetDefaultVariant(this->renderer.renderPasses[0], this->renderer.specs.multisamples) },
			{ "skybox", Vulkan::PipelineVariants::GetSkyboxVariant(this->renderer.renderPasses[0], this->renderer.specs.multisamples) },
			{ "depth", Vulkan::PipelineVariants::GetShadowVariant(this->renderer.renderPasses[2]) }, // Shadowmap
			{ "post_processing", Vulkan::PipelineVariants::GetPostProcessingVariant(this->renderer.instance.GetSurface(0).GetSwapchain().GetRenderPass())},
			{ "depth", Vulkan::PipelineVariants::GetDepthPrepassVariant(this->renderer.renderPasses[1], this->renderer.specs.multisamples) } // Depth pre-pass
		};

		for (const auto& shader : shaderList)
		{
			this->renderer.pipelines.insert(this->renderer.deviceRef->CreatePipelines(
				cs_std::binary_file(shaderPath + shader.name + ".vert.spv").open().read_if_exists(),
				cs_std::binary_file(shaderPath + shader.name + ".frag.spv").open().read_if_exists(),
				shader.variants
			));
		}

		/* ---------------------------------------------------------------- 1.1 - Descriptor Data ---------------------------------------------------------------- */

		this->renderer.pipelines[0].CreateDescriptorSets(0, 3); // One per frame in flight (fif)
		this->renderer.pipelines[0].CreateDescriptorSets(2, 3); // One per fif
		this->renderer.pipelines[0].CreateDescriptorSets(3, 3); // One per fif
		this->renderer.pipelines[1].CreateDescriptorSets(0, 3); // One per fif
		this->renderer.pipelines[2].CreateDescriptorSets(0, 3); // One per fif
		this->renderer.pipelines[4].CreateDescriptorSets(0, 3); // One per fif

		/* ---------------------------------------------------------------- 1.2 - Mesh data ---------------------------------------------------------------- */

		{
			cs_std::graphics::model skyboxModel {};
			Construct::Mesh skybox = Construct::SkyboxSphere(16, 16);
			cs_std::graphics::mesh skyboxMesh {};
			skyboxMesh.indices = skybox.indices;
			skyboxMesh.add_attribute(cs_std::graphics::Attribute::POSITION, skybox.vertices)
				.add_attribute(cs_std::graphics::Attribute::TEXCOORD_0, skybox.textureUVs);
			cs_std::graphics::mesh_attributes skyboxAttributes = cs_std::graphics::mesh_attributes().set_diffuse("./assets/skybox-night.png");
			skyboxModel.add_mesh(skyboxMesh, skyboxAttributes);

			uint32_t meshID = this->renderer.UploadMesh(skyboxMesh);
			uint32_t textureID = this->renderer.UploadTexture(LoadImage("./assets/skybox-night.png"), Colorspace::SRGB, true);

			Entity skyboxEntity = entityManager.CreateEntity();
			skyboxEntity.EmplaceComponent<Name>("Skybox");
			skyboxEntity.EmplaceComponent<Skybox>(meshID, textureID);
			skyboxEntityIdx = entities.insert(skyboxEntity);
		}

		std::string assetPath = CVar::Get<std::string>("pc_assetpath");

		cs_std::xml::document modelsXML(cs_std::text_file("./configs/models.xml").open().read());
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

		entityManager.ForEach<Behaviours>([&](entt::entity e, Behaviours& b) {
			b.OnUpdate(dt);
		});

		std::vector<DirectionalLight::ShaderRepresentation> directionalLights;
		std::vector<PointLight::ShaderRepresentation> pointLights;
		std::vector<SpotLight::ShaderRepresentation> spotLights;

		entityManager.ForEach<Transform, DirectionalLight>([&](entt::entity e, Transform& transform, DirectionalLight& light) {
			// Data packing
			if (directionalLights.size() >= this->renderer.SSBOs[this->directionalLightSSBOIdx[this->renderer.frameIndex]].size / sizeof(DirectionalLight::ShaderRepresentation)) return;
			directionalLights.push_back(light.CreateShaderRepresentation(transform));
		});

		entityManager.ForEach<Transform, PointLight>([&](entt::entity e, Transform& transform, PointLight& light) {
			// Data packing
			if (pointLights.size() >= this->renderer.SSBOs[this->pointLightSSBOIdx[this->renderer.frameIndex]].size / sizeof(PointLight::ShaderRepresentation)) return;
			pointLights.push_back(light.CreateShaderRepresentation(transform));
		});

		entityManager.ForEach<Transform, SpotLight>([&](entt::entity e, Transform& transform, SpotLight& light) {
			// Data packing
			if (spotLights.size() >= this->renderer.SSBOs[this->spotLightSSBOIdx[this->renderer.frameIndex]].size / sizeof(SpotLight::ShaderRepresentation)) return;
			spotLights.push_back(light.CreateShaderRepresentation(transform));
		});

		this->renderer.SSBOs[this->directionalLightSSBOIdx[this->renderer.frameIndex]].buffer.Fill(0, directionalLights.data(), sizeof(DirectionalLight::ShaderRepresentation) * directionalLights.size());
		this->renderer.SSBOs[this->pointLightSSBOIdx[this->renderer.frameIndex]].buffer.Fill(0, pointLights.data(), sizeof(PointLight::ShaderRepresentation) * pointLights.size());
		this->renderer.SSBOs[this->spotLightSSBOIdx[this->renderer.frameIndex]].buffer.Fill(0, spotLights.data(), sizeof(SpotLight::ShaderRepresentation) * spotLights.size());

		/* ---------------------------------------------------------------- Render preparation ---------------------------------------------------------------- */

		Vulkan::Pipelines& defaultPipeline = this->renderer.pipelines[0];
		Vulkan::Pipelines& skyboxPipeline = this->renderer.pipelines[1];
		Vulkan::Pipelines& shadowPipeline = this->renderer.pipelines[2];
		Vulkan::Pipelines& postProcessingPipeline = this->renderer.pipelines[3];
		Vulkan::Pipelines& depthPrepassPipeline = this->renderer.pipelines[4];

		Transform& cameraTransform = entities[activeCameraIdx].GetComponent<Transform>();
		const math::mat4 cameraProjection = entities[activeCameraIdx].GetComponent<PerspectiveCamera>().GetViewProjectionMatrix(cameraTransform.GetCameraViewMatrix());

		struct LightCountsDescriptor { uint32_t directional, point, spot; } lightCounts{ directionalLights.size(), pointLights.size(), spotLights.size() };

		// Arguments in order: set index, set, binding, data
		defaultPipeline.UpdateDescriptorData(this->renderer.frameIndex, 0, 0, cameraProjection);
		defaultPipeline.UpdateDescriptorData(this->renderer.frameIndex, 2, 0, cameraTransform.GetPosition());
		defaultPipeline.UpdateDescriptorData(this->renderer.frameIndex, 3, 0, lightCounts);
		skyboxPipeline.UpdateDescriptorData(this->renderer.frameIndex, 0, 0, math::translate(cameraProjection, cameraTransform.GetPosition()));
		shadowPipeline.UpdateDescriptorData(this->renderer.frameIndex, 0, 0, cameraProjection[1]);
		depthPrepassPipeline.UpdateDescriptorData(this->renderer.frameIndex, 0, 0, cameraProjection);

		/* ---------------------------------------------------------------- Render commands ---------------------------------------------------------------- */

		Vulkan::RenderCommandQueue& cur = this->renderer.GetCurrentRenderCommandQueue();
		Vulkan::GraphicsCommandQueue& cmd = cur.cmd;
		Vulkan::Surface& surface = this->renderer.instance.GetSurface(0);
		Vulkan::Swapchain& swapchain = surface.GetSwapchain();

		cmd.WaitCompletion();
		cmd.Reset();

		// Get the next image
		uint32_t currentImage = swapchain.AcquireNextImage(cur.presentReady);
		if (swapchain.NeedsRecreation())
		{
			// noop the current frame
			cmd.Begin();
			cmd.End();
			cmd.Submit(cur.presentReady, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, cur.renderFinish);
			cmd.Present(swapchain, currentImage, cur.renderFinish);
			cmd.WaitCompletion();
			cmd.Reset();
			surface.RecreateSwapchain();
			currentImage = swapchain.AcquireNextImage(cur.presentReady);
		}

		cs_std::graphics::frustum cameraFrustum(cameraProjection);

		// Meshes to render
		struct RenderData { MeshData* mesh; Material* material; };
		std::vector<RenderData> shadowMapRenderData = {}, solidRenderData = {}, transparentRenderData = {};

		entityManager.registry.view<MeshData, Material>().each([&](auto& meshData, auto& meshMaterial) {
			// Shadow map culling
			//if (meshMaterial.isShadowCasting && shadowMapFrustum.intersects(meshData.bounds))
			//{
			//	shadowMapRenderData.emplace_back(&meshData, &meshMaterial);
			//}
			// Camera culling
			if (cameraFrustum.intersects(meshData.bounds))
			{
				if (meshMaterial.isTransparent) transparentRenderData.emplace_back(&meshData, &meshMaterial);
				else solidRenderData.emplace_back(&meshData, &meshMaterial);
			}
		});

		cmd.Begin();
		// Shadowmap pass for directional lights
		{
			//const VkRect2D scissor = shadow.GetScissor();
			//cmd.DynamicStateSetViewport(shadow.GetViewport());
			//cmd.DynamicStateSetScissor(scissor);
			//cmd.BeginRenderPass(shadow.renderPass, shadow, scissor, { Vulkan::Create::DefaultDepthClear(1.0f) });
			//cmd.BindPipeline(shadowPipeline[0]);
			//cmd.BindDescriptorSets(shadowPipeline, { shadowPipeline.descriptorSets[0][this->renderer.frameIndex].set }, { 0 });
			//cmd.BindDescriptorSet(shadowPipeline, this->renderer.SSBOs[this->transformSSBOIdx[this->renderer.frameIndex]].set, 0, 1);
			//for (auto& renderData : shadowMapRenderData)
			//{
			//	const Vulkan::Mesh& mesh = this->renderer.meshes[renderData.mesh->meshID];
			//	std::vector<VkBuffer> buffers = shadowPipeline.GetMatchingBuffers(mesh);
			//	const std::vector<VkDeviceSize> bufferOffsets(buffers.size(), 0);
			//	cmd.BindVertexBuffers(buffers, bufferOffsets);
			//	cmd.BindIndexBuffer(mesh.indexBuffer);
			//	cmd.DrawIndexed(mesh.indexCount, 1, 0, 0, renderData.mesh->meshID);
			//}
			//cmd.EndRenderPass();
		}
		// Shadowmap pass for point lights
		{

		}
		// Shadowmap pass for spot lights
		{

		}
		// Depth pre-pass
		{
			const Vulkan::Framebuffer& depthPrepass = this->renderer.framebuffers[this->renderer.samplableFramebuffers[this->renderer.depthPrepassIdx].framebufferIndex];
			const VkRect2D scissor = depthPrepass.GetScissor();
			cmd.DynamicStateSetViewport(depthPrepass.GetViewport(true));
			cmd.DynamicStateSetScissor(scissor);
			cmd.BeginRenderPass(depthPrepass.renderPass, depthPrepass, scissor, { Vulkan::Create::DefaultDepthClear() });
			// Normal rendering
			{
				for (auto& renderData : solidRenderData)
				{
					const Vulkan::Mesh& mesh = this->renderer.meshes[renderData.mesh->meshID];
					cmd.BindPipeline(depthPrepassPipeline[renderData.material->isDoubleSided]);
					cmd.BindDescriptorSets(depthPrepassPipeline, {
						depthPrepassPipeline.descriptorSets[0][this->renderer.frameIndex].set,
						this->renderer.SSBOs[this->transformSSBOIdx[this->renderer.frameIndex]].set,
					}, 0, { 0 });
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
			const Vulkan::Framebuffer& offScreen = this->renderer.framebuffers[this->renderer.samplableFramebuffers[this->renderer.offscreenIdx].framebufferIndex];
			const VkRect2D scissor = offScreen.GetScissor();
			cmd.DynamicStateSetViewport(offScreen.GetViewport(true));
			cmd.DynamicStateSetScissor(scissor);
			cmd.BeginRenderPass(offScreen.renderPass, offScreen, scissor, { { 0.0f, 0.0f, 0.0f, 1.0f } });
			// Skybox rendering
			{
				Skybox& skyboxData = entities[skyboxEntityIdx].GetComponent<Skybox>();

				cmd.BindPipeline(skyboxPipeline[0]);
				cmd.BindDescriptorSets(skyboxPipeline, {
					skyboxPipeline.descriptorSets[0][this->renderer.frameIndex].set,
					this->renderer.textures[skyboxData.textureID].set
				}, 0, { 0 });
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
					cmd.BindDescriptorSets(defaultPipeline, {
						defaultPipeline.descriptorSets[0][this->renderer.frameIndex].set,
						this->renderer.SSBOs[this->transformSSBOIdx[this->renderer.frameIndex]].set,
						defaultPipeline.descriptorSets[2][this->renderer.frameIndex].set,
						defaultPipeline.descriptorSets[3][this->renderer.frameIndex].set,
						this->renderer.SSBOs[this->directionalLightSSBOIdx[this->renderer.frameIndex]].set,
						this->renderer.SSBOs[this->pointLightSSBOIdx[this->renderer.frameIndex]].set,
						this->renderer.SSBOs[this->spotLightSSBOIdx[this->renderer.frameIndex]].set,
						this->renderer.textures[renderData.material->diffuseID].set,
						this->renderer.textures[renderData.material->normalID].set,
						// this->renderer.textures[this->renderer.samplableFramebuffers[this->shadowMapIdx].textureIndices[0]].set,
					}, 0, { 0, 0, 0 });
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
					cmd.BindDescriptorSets(defaultPipeline, {
						defaultPipeline.descriptorSets[0][this->renderer.frameIndex].set,
						this->renderer.SSBOs[this->transformSSBOIdx[this->renderer.frameIndex]].set,
						defaultPipeline.descriptorSets[2][this->renderer.frameIndex].set,
						defaultPipeline.descriptorSets[3][this->renderer.frameIndex].set,
						this->renderer.SSBOs[this->directionalLightSSBOIdx[this->renderer.frameIndex]].set,
						this->renderer.SSBOs[this->pointLightSSBOIdx[this->renderer.frameIndex]].set,
						this->renderer.SSBOs[this->spotLightSSBOIdx[this->renderer.frameIndex]].set,
						this->renderer.textures[renderData.material->diffuseID].set,
						this->renderer.textures[renderData.material->normalID].set,
						// this->renderer.textures[this->renderer.samplableFramebuffers[this->shadowMapIdx].textureIndices[0]].set,
					}, 0, { 0, 0, 0 });
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
			const VkRect2D scissor = Vulkan::Create::Rect2D({ 0, 0 }, swapchain.GetExtent());
			cmd.DynamicStateSetViewport(Vulkan::Create::Viewport(0.0f, 0.0f, static_cast<float>(swapchain.GetExtent().width), static_cast<float>(swapchain.GetExtent().height), 0.0f, 1.0f));
			cmd.DynamicStateSetScissor(scissor);
			cmd.BeginRenderPass(swapchain.GetRenderPass(), swapchain.GetFramebuffer(currentImage), scissor, {{0.0f, 0.0f, 0.0f, 1.0f}});
			cmd.BindPipeline(postProcessingPipeline[0]);
			cmd.BindDescriptorSet(postProcessingPipeline, this->renderer.textures[this->renderer.samplableFramebuffers[this->renderer.offscreenIdx].textureIndices[0]].set, 0, 0, false);
			cmd.Draw(6);
			cmd.EndRenderPass();
		}
		cmd.End();
		cmd.Submit(cur.presentReady, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, cur.renderFinish);

		/* ---------------------------------------------------------------- Post-render and present ---------------------------------------------------------------- */

		cmd.Present(swapchain, currentImage, cur.renderFinish);
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
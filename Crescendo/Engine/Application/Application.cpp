#include "Application.hpp"

#include "Engine/layers/Update.hpp"
#include "Engine/CVar/Cvar.hpp"

#include "cs_std/file.hpp"

#include "Rendering/Vulkan/Create.hpp"

#include "Libraries/Construct/Construct.hpp"

#include "Assets/ImageLoader/ImageLoaders.hpp"

#define CS_STARTING_OBJECT_COUNT 1024
#define CS_STARTING_DIRECTIONAL_LIGHT_COUNT 8
#define CS_STARTING_POINT_LIGHT_COUNT 8
#define CS_STARTING_SPOT_LIGHT_COUNT 8
#define CS_STARTING_PARTICLE_COUNT 1024

CS_NAMESPACE_BEGIN
{
	struct MainPassRenderData
	{
		Vulkan::MeshHandle mesh;
		Vulkan::TextureHandle diffuse;
		Vulkan::TextureHandle normal;
		uint32_t modelID;
		bool doubleSided;
	};

	struct ParticleEmitterRenderData
	{
		uint32_t startIdx, count;
		Vulkan::TextureHandle texture;
		uint32_t modelID;
	};

	template<typename T>
	Vulkan::BufferHandle ResizeBufferIfNecessary(Vulkan::ResourceManager& resourceManager, Vulkan::BufferHandle bufferHandle, size_t requiredElementCount)
	{
		Vulkan::Buffer& buffer = resourceManager.GetBuffer(bufferHandle);
		uint32_t bufferElementCount = buffer.buffer.GetElementCount<T>();
		// 1.5x increase
		while (bufferElementCount < requiredElementCount)
		{
			bufferElementCount += bufferElementCount / 2;
		}
		if (bufferElementCount > buffer.buffer.GetElementCount<T>())
		{
			cs_std::console::info("Resizing buffer: ", buffer.buffer.GetElementCount<T>(), " -> ", bufferElementCount, " (", requiredElementCount, ")");
			resourceManager.DestroyBuffer(bufferHandle);
			return resourceManager.CreateBuffer(sizeof(T) * bufferElementCount, VK_SHADER_STAGE_ALL);
		}
		// If resize wasn't necessary, return the original buffer
		return bufferHandle;
	}

	static bool isFirstWindow = true;
	// Assign self and null as no instance exists yet
	Application* Application::self = nullptr;
	Application::Application(const ApplicationCommandLineArgs& args) : isRunning(true), shouldRestart(false), taskQueue(cs_std::task_queue()), timestamp(), layerManager(LayerStack())
	{
		self = this;
		// Either use the default config, which should be in the same directory as the executable, named "config.xml" or can be specified in the command line
		std::string config = args.HasArg("config") ? args.GetArg("config") : "config.xml";
		if (cs_std::text_file(config).exists()) CVar::LoadConfigXML(config);

		this->CreateDefaultWindow();

		this->instance = Vulkan::Instance({
			CVar::Get<bool>("irc_validationlayers"),
			CVar::Get<std::string>("ec_appname"), "Crescendo",
		});
		this->instance.CreateSurface(this->GetWindow()->GetNative(), {
			.swapchainRecreationCallback = nullptr
		});
		this->frameManager = Vulkan::FrameManager(this->instance.GetSurface(0).GetDevice(), CVar::Get<uint32_t>("rc_framesinflight"));
		this->resourceManager = Vulkan::ResourceManager(this->instance.GetSurface(0).GetDevice(), {
				CVar::Get<uint32_t>("irc_maxbufferdescriptors_bindless"),
				CVar::Get<uint32_t>("irc_maxtexturedescriptors_bindless")
		});

		// Create a default scene
		loadedScenes.emplace_back();
		activeScene = &loadedScenes[0];

		{
			Vulkan::Surface& surface = this->instance.GetSurface(0);
			Vulkan::Vk::Swapchain& swapchain = surface.GetSwapchain();
			Vulkan::Vk::Device& device = surface.GetDevice();

			// HDR
			constexpr VkFormat COLOR_FORMAT = VK_FORMAT_B10G11R11_UFLOAT_PACK32;
			constexpr VkFormat DEPTH_FORMAT = VK_FORMAT_D32_SFLOAT;
			constexpr VkSampleCountFlagBits MULTISAMPLES = VK_SAMPLE_COUNT_1_BIT;
			const double RENDER_SCALE = CVar::Get<double>("rc_renderscale");

			depthRenderPass = Vulkan::Vk::RenderPass::CreateReversedZDepthRenderPass(device, DEPTH_FORMAT, MULTISAMPLES);
			mainRenderPass = Vulkan::Vk::RenderPass::CreateMainRenderPass(device, COLOR_FORMAT, DEPTH_FORMAT, MULTISAMPLES);

			resourceManager.DestroyTexture(depthImageHandle);
			depthImageHandle = resourceManager.CreateTexture(
				Vulkan::Create::ImageCreateInfo(
					VK_IMAGE_TYPE_2D, DEPTH_FORMAT, Vulkan::Create::Extent3D(swapchain.GetExtent3D(), RENDER_SCALE),
					1, 1, MULTISAMPLES, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
				),
				Vulkan::Create::AllocationCreateInfo(VMA_MEMORY_USAGE_GPU_ONLY)
			);
			const VkImageView depthImageView = resourceManager.GetTexture(depthImageHandle).image.GetImageView();
			depthFramebuffer = Vulkan::Vk::Framebuffer(device, Vulkan::Create::FramebufferCreateInfo(
				depthRenderPass, &depthImageView, Vulkan::Create::Extent2D(swapchain.GetExtent(), RENDER_SCALE), 1
			));

			resourceManager.DestroyTexture(mainImageHandle);
			mainImageHandle = resourceManager.CreateTexture(
				Vulkan::Create::ImageCreateInfo(
					VK_IMAGE_TYPE_2D, COLOR_FORMAT, Vulkan::Create::Extent3D(swapchain.GetExtent3D(), RENDER_SCALE),
					1, 1, MULTISAMPLES, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
				),
				Vulkan::Create::AllocationCreateInfo(VMA_MEMORY_USAGE_GPU_ONLY)
			);
			const std::array<VkImageView, 2> attachments2 = { resourceManager.GetTexture(mainImageHandle).image, depthImageView };
			mainFramebuffer = Vulkan::Vk::Framebuffer(device, Vulkan::Create::FramebufferCreateInfo(
				mainRenderPass, attachments2, Vulkan::Create::Extent2D(swapchain.GetExtent(), RENDER_SCALE), 1
			));
		}
		{
			Vulkan::Surface& surface = this->instance.GetSurface(0);
			Vulkan::Vk::Swapchain& swapchain = surface.GetSwapchain();
			Vulkan::Vk::Device& device = surface.GetDevice();

			const std::string postProcessingShader = CVar::Get<std::string>("ircs_postprocessing");
			const std::string depthPrepassShader = CVar::Get<std::string>("ircs_depthprepass");
			const std::string mainShader = CVar::Get<std::string>("ircs_main");
			const std::string skyboxShader = CVar::Get<std::string>("ircs_skybox");
			const std::string particleShader = CVar::Get<std::string>("ircs_particle");

			postProcessingPipeline = Vulkan::Vk::Pipeline(device, {
				cs_std::binary_file(postProcessingShader + ".vert.spv").open().read_if_exists(),
				cs_std::binary_file(postProcessingShader + ".frag.spv").open().read_if_exists(),
				Vulkan::Vk::PipelineVariants::GetPostProcessingVariant(),
				resourceManager.GetDescriptorSetLayout(),
				swapchain.GetRenderPass()
			});
			depthPipeline = Vulkan::Vk::Pipeline(device, {
				cs_std::binary_file(depthPrepassShader + ".vert.spv").open().read_if_exists(), {},
				Vulkan::Vk::PipelineVariants::GetDepthPrepassVariant(),
				resourceManager.GetDescriptorSetLayout(), depthRenderPass
			});
			mainPipeline = Vulkan::Vk::Pipeline(device, {
				cs_std::binary_file(mainShader + ".vert.spv").open().read_if_exists(),
				cs_std::binary_file(mainShader + ".frag.spv").open().read_if_exists(),
				Vulkan::Vk::PipelineVariants::GetDefaultVariant(),
				resourceManager.GetDescriptorSetLayout(), mainRenderPass
			});
			skyboxPipeline = Vulkan::Vk::Pipeline(device, {
				cs_std::binary_file(skyboxShader + ".vert.spv").open().read_if_exists(),
				cs_std::binary_file(skyboxShader + ".frag.spv").open().read_if_exists(),
				Vulkan::Vk::PipelineVariants::GetSkyboxVariant(),
				resourceManager.GetDescriptorSetLayout(), mainRenderPass
			});
			particlePipeline = Vulkan::Vk::Pipeline(device, {
				cs_std::binary_file(particleShader + ".vert.spv").open().read_if_exists(),
				cs_std::binary_file(particleShader + ".frag.spv").open().read_if_exists(),
				Vulkan::Vk::PipelineVariants::GetParticleVariant(),
				resourceManager.GetDescriptorSetLayout(), mainRenderPass
			});

			// Skybox
			Construct::Mesh skybox = Construct::SkyboxSphere(32, 32);
			cs_std::graphics::mesh skyboxMesh;
			skyboxMesh.add_attribute(cs_std::graphics::Attribute::POSITION, skybox.vertices);
			skyboxMesh.add_attribute(cs_std::graphics::Attribute::NORMAL, skybox.normals);
			skyboxMesh.add_attribute(cs_std::graphics::Attribute::TEXCOORD_0, skybox.textureUVs);
			skyboxMesh.indices = skybox.indices;

			skyboxMeshHandle = resourceManager.UploadMesh(skyboxMesh);
			if (CVar::Exists("skybox_texture"))
			{
				skyboxTextureHandle = resourceManager.UploadTexture(LoadImage(CVar::Get<std::string>("skybox_texture")), { Vulkan::ResourceManager::Colorspace::SRGB, 1.0f, true });
			}

			for (uint32_t i = 0; i < frameManager.GetFrameCount(); i++)
			{
				// Create transforms buffer
				transformsHandle.push_back(resourceManager.CreateBuffer(sizeof(cs_std::math::mat4) * CS_STARTING_OBJECT_COUNT, VK_SHADER_STAGE_ALL));

				// Create lights buffer
				directionalLightsHandle.push_back(resourceManager.CreateBuffer(sizeof(DirectionalLight::ShaderRepresentation) * CS_STARTING_DIRECTIONAL_LIGHT_COUNT, VK_SHADER_STAGE_ALL));
				pointLightsHandle.push_back(resourceManager.CreateBuffer(sizeof(PointLight::ShaderRepresentation) * CS_STARTING_POINT_LIGHT_COUNT, VK_SHADER_STAGE_ALL));
				spotLightsHandle.push_back(resourceManager.CreateBuffer(sizeof(SpotLight::ShaderRepresentation) * CS_STARTING_SPOT_LIGHT_COUNT, VK_SHADER_STAGE_ALL));

				// Create particle buffer
				particleBufferHandle.push_back(resourceManager.CreateBuffer(sizeof(ParticleEmitter::ParticleShaderRepresentation) * CS_STARTING_PARTICLE_COUNT, VK_SHADER_STAGE_ALL));
			}
		}
	}
	Application::~Application()
	{
		this->instance.GetSurface(0).GetDevice().WaitIdle();
	}
	void Application::InternalUpdate(double dt)
	{

		// Commonly used resources
		Vulkan::Surface& surface = this->instance.GetSurface(0);
		Vulkan::Vk::Swapchain& swapchain = surface.GetSwapchain();
		Vulkan::FrameResources& frame = this->frameManager.GetCurrentFrame();
		uint32_t currentFrameIndex = this->frameManager.GetCurrentFrameIndex();
		Vulkan::Vk::GraphicsCommandQueue& cmd = frame.commandBuffer;

		// Active scene
		Scene& currentScene = *activeScene;

		// Active camera in the scene
		Entity camera = currentScene.entityManager.GetEntity(currentScene.activeCamera);
		if (!camera.IsValid()) return;

		cs_std::math::vec3 cameraPosition = camera.GetComponent<Transform>().GetPosition();
		cs_std::math::mat4 cameraView = camera.GetComponent<Transform>().GetCameraViewMatrix();
		cs_std::math::mat4 cameraProjection = camera.GetComponent<PerspectiveCamera>().GetProjectionMatrix(this->GetWindow()->GetAspectRatio());
		cs_std::math::mat4 cameraViewProjection = cameraProjection * cameraView;
		cs_std::graphics::frustum cameraFrustum(cameraViewProjection);

		std::vector<DirectionalLight::ShaderRepresentation> directionalLights;
		std::vector<PointLight::ShaderRepresentation> pointLights;
		std::vector<SpotLight::ShaderRepresentation> spotLights;

		std::vector<cs_std::math::mat4> meshTransforms;
		std::deque<MainPassRenderData> meshes;
		uint32_t depthPrepassMeshCount = 0;

		std::vector<ParticleEmitter::ParticleShaderRepresentation> particles;
		std::vector<ParticleEmitterRenderData> particleEmitters;

		// Add active camera matrices to the buffer
		meshTransforms.push_back(cameraView); // Used for billboarding
		meshTransforms.push_back(cameraProjection);  // Used for billboarding
		meshTransforms.push_back(cameraViewProjection);

		// Update entity behaviours
		currentScene.entityManager.ForEach<Behaviours>([&](Behaviours& b) {
			b.OnUpdate(dt);
		});

		// Add lights to the buffer
		currentScene.entityManager.ForEach<Transform, DirectionalLight>([&](Transform& transform, DirectionalLight& light) {
			directionalLights.push_back(light.CreateShaderRepresentation(transform));
		});
		currentScene.entityManager.ForEach<Transform, PointLight>([&](Transform& transform, PointLight& light) {
			pointLights.push_back(light.CreateShaderRepresentation(transform));
		});
		currentScene.entityManager.ForEach<Transform, SpotLight>([&](Transform& transform, SpotLight& light) {
			spotLights.push_back(light.CreateShaderRepresentation(transform));
		});

		// Update particle emitters and gather render data
		currentScene.entityManager.ForEach<Transform, ParticleEmitter>([&](Transform& transform, ParticleEmitter& emitter) {
			emitter.Update(GetTime<float>(), dt);

			meshTransforms.push_back(transform.GetModelMatrix());
			particleEmitters.emplace_back(particles.size(), emitter.liveParticleCount, emitter.texture, meshTransforms.size() - 1);
			for (const ParticleEmitter::Particle& particle : emitter.particles)
			{
				particles.push_back(particle.CreateShaderRepresentation());
			}
		});

		currentScene.entityManager.ForEach<Transform, MeshData, Material>([&](Transform& transform, MeshData& mesh, Material& material) {
			// frustum cull
			//if (!cameraFrustum.intersects(mesh.bounds)) return;
			meshTransforms.push_back(transform.GetModelMatrix());
			MainPassRenderData data(
				mesh.meshHandle, material.diffuseHandle, material.normalHandle, meshTransforms.size() - 1, material.isDoubleSided
			);
			if (material.isTransparent)
			{
				meshes.push_back(data);
			}
			else
			{
				meshes.push_front(data);
				depthPrepassMeshCount++;
			}

		});

		// Only sort the transparent meshes
		std::sort(meshes.begin() + depthPrepassMeshCount, meshes.end(), [&](const MainPassRenderData& a, const MainPassRenderData& b) {
			// use the meshTransforms to get it's position in the scene
			float depthA = cs_std::math::distance(cameraPosition, cs_std::math::vec3(meshTransforms[a.modelID][3]));
			float depthB = cs_std::math::distance(cameraPosition, cs_std::math::vec3(meshTransforms[b.modelID][3]));
			return depthA > depthB;
		});

		cmd.WaitCompletion();

		// Resize buffers if required
		transformsHandle[currentFrameIndex] = ResizeBufferIfNecessary<cs_std::math::mat4>(resourceManager, transformsHandle[currentFrameIndex], meshTransforms.size());
		directionalLightsHandle[currentFrameIndex] = ResizeBufferIfNecessary<DirectionalLight::ShaderRepresentation>(resourceManager, directionalLightsHandle[currentFrameIndex], directionalLights.size());
		pointLightsHandle[currentFrameIndex] = ResizeBufferIfNecessary<PointLight::ShaderRepresentation>(resourceManager, pointLightsHandle[currentFrameIndex], pointLights.size());
		spotLightsHandle[currentFrameIndex] = ResizeBufferIfNecessary<SpotLight::ShaderRepresentation>(resourceManager, spotLightsHandle[currentFrameIndex], spotLights.size());
		particleBufferHandle[currentFrameIndex] = ResizeBufferIfNecessary<ParticleEmitter::ParticleShaderRepresentation>(resourceManager, particleBufferHandle[currentFrameIndex], particles.size());

		// Write data to buffers
		resourceManager.GetBuffer(transformsHandle[currentFrameIndex]).buffer.memcpy(meshTransforms.data(), sizeof(cs_std::math::mat4) * meshTransforms.size());
		resourceManager.GetBuffer(directionalLightsHandle[currentFrameIndex]).buffer.memcpy(directionalLights.data(), sizeof(DirectionalLight::ShaderRepresentation) * directionalLights.size());
		resourceManager.GetBuffer(pointLightsHandle[currentFrameIndex]).buffer.memcpy(pointLights.data(), sizeof(PointLight::ShaderRepresentation) * pointLights.size());
		resourceManager.GetBuffer(spotLightsHandle[currentFrameIndex]).buffer.memcpy(spotLights.data(), sizeof(SpotLight::ShaderRepresentation) * spotLights.size());
		resourceManager.GetBuffer(particleBufferHandle[currentFrameIndex]).buffer.memcpy(particles.data(), sizeof(ParticleEmitter::ParticleShaderRepresentation) * particles.size());

		cmd.Reset();

		surface.AcquireNextImage(frame.imageAvailable);

		cmd.Begin();

		// Depth prepass
		cmd.DynamicStateSetViewport(depthFramebuffer.GetViewport());
		cmd.DynamicStateSetScissor(depthFramebuffer.GetScissor());

		cmd.BeginRenderPass(depthRenderPass, depthFramebuffer, depthFramebuffer.GetScissor(), Vulkan::Create::DefaultDepthClear());
		cmd.BindPipeline(depthPipeline);
		cmd.BindDescriptorSet(depthPipeline, resourceManager.GetDescriptorSet(), 0, 0, false);

		const struct DepthPrepassParams {
			uint32_t cameraIdx, transformBufferIdx;
		} depthPrepassParams { 2, transformsHandle[currentFrameIndex].GetIndex()};
		cmd.PushConstants(depthPipeline, depthPrepassParams, VK_SHADER_STAGE_VERTEX_BIT);
 
		for (size_t i = 0; i < depthPrepassMeshCount; i++)
		{
			MainPassRenderData& data = meshes[i];
			cmd.BindPipeline(depthPipeline[data.doubleSided]);
			Vulkan::Mesh& mesh = resourceManager.GetMesh(data.mesh);
			cmd.BindIndexBuffer(mesh.indexBuffer);
			cmd.BindVertexBuffers({ *mesh.GetAttributeBuffer(cs_std::graphics::Attribute::POSITION) }, { 0 });
			cmd.DrawIndexed(mesh.indexCount, 1, 0, 0, data.modelID);
		}
		cmd.EndRenderPass();

		// Main pass
		cmd.DynamicStateSetViewport(mainFramebuffer.GetViewport());
		cmd.DynamicStateSetScissor(mainFramebuffer.GetScissor());

		cmd.BeginRenderPass(mainRenderPass, mainFramebuffer, mainFramebuffer.GetScissor(), { Vulkan::Create::ClearValue(0.0f, 0.0f, 0.0f, 1.0f) });

		// Render skybox
		if (skyboxTextureHandle.IsValid())
		{
			cmd.BindPipeline(skyboxPipeline);
			cmd.BindDescriptorSet(skyboxPipeline, resourceManager.GetDescriptorSet(), 0, 0, false);

			const struct SkyboxParams {
				uint32_t cameraIdx, transformBufferIdx;
			} skyboxParams{ 2, transformsHandle[currentFrameIndex].GetIndex() };
			const uint32_t skyboxTexture = skyboxTextureHandle.GetIndex();
			cmd.PushConstants(skyboxPipeline, skyboxParams, VK_SHADER_STAGE_VERTEX_BIT);
			cmd.PushConstants(skyboxPipeline, skyboxTexture, VK_SHADER_STAGE_FRAGMENT_BIT, 2 * sizeof(uint32_t));
			cmd.BindIndexBuffer(resourceManager.GetMesh(skyboxMeshHandle).indexBuffer);
			cmd.BindVertexBuffers({
				*resourceManager.GetMesh(skyboxMeshHandle).GetAttributeBuffer(cs_std::graphics::Attribute::POSITION),
				*resourceManager.GetMesh(skyboxMeshHandle).GetAttributeBuffer(cs_std::graphics::Attribute::TEXCOORD_0),
				}, { 0, 0 });
			cmd.DrawIndexed(resourceManager.GetMesh(skyboxMeshHandle).indexCount, 1, 0, 0, 0);
			cmd.BindDescriptorSet(mainPipeline, resourceManager.GetDescriptorSet(), 0, 0, false);
		}

		// main pass has two push constants, one in the vertex shader, one in the fragment shader
		const struct MainPassVertexParams {
			uint32_t cameraIdx, transformBufferIdx;
		} mainPassParams { 2, transformsHandle[currentFrameIndex].GetIndex()};
		cmd.PushConstants(mainPipeline, mainPassParams, VK_SHADER_STAGE_VERTEX_BIT);
		// Render objects, renders opaque first, then transparent
		for (size_t i = 0; i < meshes.size(); i++)
		{
			MainPassRenderData& data = meshes[i];
			Vulkan::Mesh& mesh = resourceManager.GetMesh(data.mesh);
			cmd.BindPipeline(mainPipeline[data.doubleSided]);
			cmd.BindIndexBuffer(mesh.indexBuffer);
			cmd.BindVertexBuffers({
				*mesh.GetAttributeBuffer(cs_std::graphics::Attribute::POSITION),
				*mesh.GetAttributeBuffer(cs_std::graphics::Attribute::NORMAL),
				*mesh.GetAttributeBuffer(cs_std::graphics::Attribute::TANGENT),
				*mesh.GetAttributeBuffer(cs_std::graphics::Attribute::TEXCOORD_0)
			}, { 0, 0, 0, 0 });
			const struct MainPassFragmentParams {
				uint32_t diffuseTexIdx, normalTexIdx;
				uint32_t directionalLightBufferIdx, pointLightBufferIdx, spotLightBufferIdx;
				uint32_t directionalLightCount, pointLightCount, spotLightCount;
				uint32_t dummy0, dummy1;
				cs_std::math::vec4 cameraViewPos;
			} texturesParams {
				data.diffuse.GetIndex(), data.normal.GetIndex(),
				directionalLightsHandle[currentFrameIndex].GetIndex(), pointLightsHandle[currentFrameIndex].GetIndex(), spotLightsHandle[currentFrameIndex].GetIndex(),
				static_cast<uint32_t>(directionalLights.size()), static_cast<uint32_t>(pointLights.size()), static_cast<uint32_t>(spotLights.size()),
				0, 0,
				cs_std::math::vec4(cameraPosition, 1.0f)
			};
			cmd.PushConstants(mainPipeline, texturesParams, VK_SHADER_STAGE_FRAGMENT_BIT, 2 * sizeof(uint32_t));
			cmd.DrawIndexed(mesh.indexCount, 1, 0, 0, data.modelID);
		}

		// Particle pass
		cmd.BindPipeline(particlePipeline);
		cmd.BindDescriptorSet(particlePipeline, resourceManager.GetDescriptorSet(), 0, 0, false);
		for (uint32_t i = 0; i < particleEmitters.size(); i++ )
		{
			ParticleEmitterRenderData& data = particleEmitters[i];
			const struct ParticleVertexParams {
				uint32_t cameraIdx, transformBufferIdx, particleBufferIdx, particleStartingIdx;
				float currentTime;
			} particleParams {
				0, transformsHandle[currentFrameIndex].GetIndex(), particleBufferHandle[currentFrameIndex].GetIndex(), data.startIdx,
				GetTime<float>()
			};
			const struct ParticleFragmentParams {
				float currentTime;
				uint32_t diffuseTexIdx;
			} particleParamsFrag {
				GetTime<float>(),
				data.texture.GetIndex()
			};
			cmd.PushConstants(particlePipeline, particleParams, VK_SHADER_STAGE_VERTEX_BIT);
			cmd.PushConstants(particlePipeline, particleParamsFrag, VK_SHADER_STAGE_FRAGMENT_BIT, 5 * sizeof(uint32_t));
			// One quad for each particle, hence 6 vertices
			cmd.Draw(data.count * 6, 1, 0, data.modelID);
		}
		cmd.EndRenderPass();


		// Final post processing pass
		cmd.DynamicStateSetViewport(swapchain.GetViewport(true));
		cmd.DynamicStateSetScissor(swapchain.GetScissor());
		cmd.BeginRenderPass(
			swapchain.GetRenderPass(), swapchain.GetFramebuffer(swapchain.GetAcquiredImageIndex()).framebuffer, swapchain.GetScissor(),
			{ Vulkan::Create::ClearValue(1.0f, 0.0f, 0.0f, 1.0f), Vulkan::Create::DefaultDepthClear() }
		);
		cmd.BindPipeline(postProcessingPipeline);
		cmd.BindDescriptorSet(postProcessingPipeline, resourceManager.GetDescriptorSet(), 0, 0, false);

		uint32_t offscreenTexIdx = mainImageHandle.GetIndex();
		cmd.PushConstants(postProcessingPipeline, offscreenTexIdx, VK_SHADER_STAGE_FRAGMENT_BIT);
		cmd.Draw(6);
		cmd.EndRenderPass();

		cmd.End();
		cmd.Submit(frame.imageAvailable, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, frame.renderFinished);

		surface.Present(cmd.GetQueue(), frame.renderFinished);

		this->frameManager.AdvanceFrame();
	}
	void Application::Run()
	{
		this->OnStartup();
		while (this->IsRunning())
		{
			const double time = this->timestamp.elapsed<double>();
			this->layerManager.QueryForUpdate(time);
			this->layerManager.Update(time);

			// Check if all windows are closed, then terminate app
			bool allWindowsClosed = true;
			for (auto& window : this->windows) allWindowsClosed &= !window->IsOpen();
			if (allWindowsClosed) this->Exit();

		}
		this->OnExit();
	}
	void Application::Exit()
	{
		for (auto& window : this->windows) window->Close();
		this->isRunning = false;
		this->shouldRestart = false;
	}
	void Application::Restart()
	{
		for (auto& window : this->windows) window->Close();
		this->isRunning = false;
		this->shouldRestart = true;
		self = nullptr;
	}
	bool Application::IsRunning() const { return this->isRunning; };
	bool Application::ShouldRestart() const { return this->shouldRestart; };
	Window* Application::GetWindow(size_t index) const { return this->windows[index].get(); };
	size_t Application::GetWindowCount() const { return this->windows.size(); };
	size_t Application::CreateWindow(const Window::Specification& info)
	{
		this->windows.push_back(Window::Create(info));

		// If this is the first window, attach the update layer
		if (isFirstWindow)
		{
			const uint32_t refreshRate = this->windows[0]->GetRefreshRate();
			const double secondsPerFrame = (refreshRate == 0 || !CVar::Get<bool>("ec_vsync")) ? 0.0 : 1.0 / double(refreshRate);
			this->layerManager.Attach(new LayerUpdate(secondsPerFrame));
			this->layerManager.Init(this->timestamp.elapsed<double>());
			isFirstWindow = false;
		}

		// Return the index of the window
		return this->windows.size() - 1;
	};
	void Application::CreateDefaultWindow()
	{
		if (this->windows.size() > 0)
		{
			cs_std::console::warn("Attempted to create a default window when one already exists");
			return;
		}
		this->CreateWindow(Window::Specification(
			CVar::Get<std::string>("ec_appname"),
			CVar::Get<uint32_t>("ec_windowwidth"),
			CVar::Get<uint32_t>("ec_windowheight")
		));
	}
	void Application::CloseWindow(size_t index)
	{
		this->windows[index]->Close();
		this->windows.erase(this->windows.begin() + index);
	}
	Scene& Application::GetActiveScene() { return *activeScene; }
	Application* Application::Get() { return self; }
}
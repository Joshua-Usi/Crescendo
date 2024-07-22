#include "Application.hpp"

#include "Engine/layers/Update.hpp"
#include "Engine/CVar/Cvar.hpp"

#include "cs_std/file.hpp"

#include "Rendering/Vulkan2/Create.hpp"

#include "Libraries/Construct/Construct.hpp"

#include "Assets/ImageLoader/ImageLoaders.hpp"

CS_NAMESPACE_BEGIN
{
	static bool isFirstWindow = true;
	// Assign self and null as no instance exists yet
	Application* Application::self = nullptr;
	Application::Application() : isRunning(true), shouldRestart(false), taskQueue(cs_std::task_queue()), timestamp(), layerManager(LayerStack())
	{
		self = this;
		CVar::LoadConfigXML("./configs/config.xml");

		this->CreateDefaultWindow();
		this->GetWindow()->SetCursorLock(true);

		Vulkan::InstanceSpecification spec {
			CVar::Get<bool>("irc_validationlayers"),
			CVar::Get<std::string>("ec_appname"), "Crescendo",
		};
		this->instance = Vulkan::Instance(spec);
		this->instance.CreateSurface(this->GetWindow()->GetNative(), {
			.descriptorManagerSpec = {
				CVar::Get<uint32_t>("irc_maxbufferdescriptors_bindless"),
				CVar::Get<uint32_t>("irc_maxtexturedescriptors_bindless")
			},
			.swapchainRecreationCallback = nullptr
		});
		this->frameManager = Vulkan::FrameManager(this->instance.GetSurface(0).GetDevice(), CVar::Get<uint32_t>("rc_framesinflight"));
		this->resourceManager = Vulkan::ResourceManager(this->instance.GetSurface(0).GetDevice());

		// Create a default scene
		loadedScenes.emplace_back();
		activeScene = &loadedScenes[0];

		{
			Vulkan::Surface& surface = this->instance.GetSurface(0);
			Vulkan::Vk::Swapchain& swapchain = surface.GetSwapchain();
			Vulkan::Device& device = surface.GetDevice();
			Vulkan::BindlessDescriptorManager& bindlessDescriptorManager = device.GetBindlessDescriptorManager();

			const std::string postProcessingShader = CVar::Get<std::string>("ircs_postprocessing");
			const std::string depthPrepassShader = CVar::Get<std::string>("ircs_depthprepass");
			const std::string mainShader = CVar::Get<std::string>("ircs_main");
			const std::string skyboxShader = CVar::Get<std::string>("ircs_skybox");

			constexpr VkFormat colorFormat = VK_FORMAT_B10G11R11_UFLOAT_PACK32;
			constexpr VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;
			constexpr VkSampleCountFlagBits multisamples = VK_SAMPLE_COUNT_1_BIT;

			postProcessingPipeline = Vulkan::Vk::Pipeline(device, {
				cs_std::binary_file(postProcessingShader + ".vert.spv").open().read_if_exists(),
				cs_std::binary_file(postProcessingShader + ".frag.spv").open().read_if_exists(),
				Vulkan::Vk::PipelineVariants::GetPostProcessingVariant(),
				device.GetBindlessDescriptorManager().GetLayout(),
				swapchain.GetRenderPass()
			});

			VkAttachmentDescription depthAttachment = Vulkan::Create::AttachmentDescription(
				depthFormat, multisamples, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
				VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
				VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
			);
			VkAttachmentReference depthAttachmentRef = Vulkan::Create::AttachmentReference(0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
			VkSubpassDescription depthSubpass = Vulkan::Create::SubpassDescription(VK_PIPELINE_BIND_POINT_GRAPHICS, nullptr, nullptr, nullptr, &depthAttachmentRef, nullptr);
			const std::array<VkSubpassDependency, 2> depthDependencies = {
				Vulkan::Create::SubpassDependency(
					VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
					VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, VK_ACCESS_SHADER_READ_BIT,
					VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_DEPENDENCY_BY_REGION_BIT
				),
				Vulkan::Create::SubpassDependency(
					0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
					VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
					VK_ACCESS_SHADER_READ_BIT, VK_DEPENDENCY_BY_REGION_BIT
				)
			};

			depthRenderPass = Vulkan::Vk::RenderPass(device, Vulkan::Create::RenderPassCreateInfo(&depthAttachment, &depthSubpass, depthDependencies));
			depthPipeline = Vulkan::Vk::Pipeline(device, {
				cs_std::binary_file(depthPrepassShader + ".vert.spv").open().read_if_exists(), {},
				Vulkan::Vk::PipelineVariants::GetDepthPrepassVariant(),
				device.GetBindlessDescriptorManager().GetLayout(), depthRenderPass
			});
			depthImage = Vulkan::Vk::Image(device, device.GetAllocator(), Vulkan::Create::ImageCreateInfo(
				VK_IMAGE_TYPE_2D, depthFormat, surface.GetSwapchain().GetExtent3D(),
				1, 1, multisamples, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
			), Vulkan::Create::AllocationCreateInfo(VMA_MEMORY_USAGE_GPU_ONLY));
			const VkImageView depthImageView = depthImage.GetImageView();
			depthFramebuffer = Vulkan::Vk::Framebuffer(device, Vulkan::Create::FramebufferCreateInfo(
				depthRenderPass, &depthImageView, swapchain.GetExtent(), 1
			));

			VkAttachmentDescription colorAttachment = Vulkan::Create::AttachmentDescription(
				colorFormat, multisamples,
				VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
				VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
				VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
			);
			VkAttachmentReference colorAttachmentRef = Vulkan::Create::AttachmentReference(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
			depthAttachment = Vulkan::Create::AttachmentDescription(
				depthFormat, multisamples,
				VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_DONT_CARE,
				VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
				VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
			);
			depthAttachmentRef = Vulkan::Create::AttachmentReference(1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
			const VkSubpassDescription subpass = Vulkan::Create::SubpassDescription(
				VK_PIPELINE_BIND_POINT_GRAPHICS, nullptr, &colorAttachmentRef, nullptr, &depthAttachmentRef, nullptr
			);
			VkSubpassDependency colorDependency = Vulkan::Create::SubpassDependency(
				VK_SUBPASS_EXTERNAL, 0,
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_DEPENDENCY_BY_REGION_BIT
			);
			VkSubpassDependency colorDependency2 = Vulkan::Create::SubpassDependency(
				0, VK_SUBPASS_EXTERNAL,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_DEPENDENCY_BY_REGION_BIT
			);
			VkSubpassDependency depthDependency = Vulkan::Create::SubpassDependency(
				VK_SUBPASS_EXTERNAL, 0,
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
				VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_DEPENDENCY_BY_REGION_BIT
			);
			std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
			std::array<VkSubpassDependency, 3> dependencies = { colorDependency, colorDependency2, depthDependency };

			mainRenderPass = Vulkan::Vk::RenderPass(device, Vulkan::Create::RenderPassCreateInfo(attachments, &subpass, dependencies));
			mainPipeline = Vulkan::Vk::Pipeline(device, {
				cs_std::binary_file(mainShader + ".vert.spv").open().read_if_exists(),
				cs_std::binary_file(mainShader + ".frag.spv").open().read_if_exists(),
				Vulkan::Vk::PipelineVariants::GetDefaultVariant(),
				device.GetBindlessDescriptorManager().GetLayout(), mainRenderPass
			});
			mainImage = Vulkan::Vk::Image(device, device.GetAllocator(), Vulkan::Create::ImageCreateInfo(
				VK_IMAGE_TYPE_2D, colorFormat, swapchain.GetExtent3D(),
				1, 1, multisamples, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
			), Vulkan::Create::AllocationCreateInfo(VMA_MEMORY_USAGE_GPU_ONLY));
			const std::array<VkImageView, 2> attachments2 = { mainImage.GetImageView(), depthImageView };
			mainFramebuffer = Vulkan::Vk::Framebuffer(device, Vulkan::Create::FramebufferCreateInfo(
				mainRenderPass, attachments2, swapchain.GetExtent(), 1
			));
			postProcessingSampler = Vulkan::Vk::Sampler(device, Vulkan::Create::SamplerCreateInfo(
				VK_FILTER_LINEAR, VK_FILTER_LINEAR,
				VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
				1.0f, 1.0f, VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK
			));

			// Skybox
			skyboxPipeline = Vulkan::Vk::Pipeline(device, {
				cs_std::binary_file(skyboxShader + ".vert.spv").open().read_if_exists(),
				cs_std::binary_file(skyboxShader + ".frag.spv").open().read_if_exists(),
				Vulkan::Vk::PipelineVariants::GetSkyboxVariant(),
				device.GetBindlessDescriptorManager().GetLayout(), mainRenderPass
			});
			Construct::Mesh skybox = Construct::SkyboxSphere(32, 32);
			cs_std::graphics::mesh skyboxMesh;
			skyboxMesh.add_attribute(cs_std::graphics::Attribute::POSITION, skybox.vertices);
			skyboxMesh.add_attribute(cs_std::graphics::Attribute::NORMAL, skybox.normals);
			skyboxMesh.add_attribute(cs_std::graphics::Attribute::TEXCOORD_0, skybox.textureUVs);
			skyboxMesh.indices = skybox.indices;
			
			skyboxMeshHandle = resourceManager.UploadMesh(skyboxMesh);
			skyboxTextureHandle = resourceManager.UploadTexture(LoadImage(CVar::Get<std::string>("skybox_texture")), { Vulkan::ResourceManager::Colorspace::SRGB, 1.0f, true });

			Vulkan::Texture& skyboxTexture = resourceManager.GetTexture(skyboxTextureHandle);
			bindlessDescriptorManager.StoreImage(mainImage.GetImageView(), postProcessingSampler);
			bindlessDescriptorManager.StoreImage(skyboxTexture.image.GetImageView(), skyboxTexture.sampler);

			const uint32_t MAX_OBJECT_COUNT = CVar::Get<uint32_t>("irc_maxobjectcount");
			const uint32_t MAX_DIRECTIONAL_LIGHT_COUNT = CVar::Get<uint32_t>("irc_maxdirectionallightcount") * 2;
			const uint32_t MAX_POINT_LIGHT_COUNT = CVar::Get<uint32_t>("irc_maxpointlightcount") * 2;
			const uint32_t MAX_SPOT_LIGHT_COUNT = CVar::Get<uint32_t>("irc_maxspotlightcount") * 2;

			for (uint32_t i = 0; i < frameManager.GetFrameCount(); i++)
			{
				// Create transforms buffer
				transformsHandle.push_back(resourceManager.CreateBuffer(sizeof(cs_std::math::mat4) * MAX_OBJECT_COUNT, VK_SHADER_STAGE_ALL));
				bindlessDescriptorManager.StoreBuffer(resourceManager.GetBuffer(transformsHandle[i]).buffer);

				// Create lights buffer
				directionalLightsHandle.push_back(resourceManager.CreateBuffer(sizeof(DirectionalLight::ShaderRepresentation) * MAX_DIRECTIONAL_LIGHT_COUNT, VK_SHADER_STAGE_ALL));
				bindlessDescriptorManager.StoreBuffer(resourceManager.GetBuffer(directionalLightsHandle[i]).buffer);

				pointLightsHandle.push_back(resourceManager.CreateBuffer(sizeof(PointLight::ShaderRepresentation) * MAX_POINT_LIGHT_COUNT, VK_SHADER_STAGE_ALL));
				bindlessDescriptorManager.StoreBuffer(resourceManager.GetBuffer(pointLightsHandle[i]).buffer);

				spotLightsHandle.push_back(resourceManager.CreateBuffer(sizeof(SpotLight::ShaderRepresentation) * MAX_SPOT_LIGHT_COUNT, VK_SHADER_STAGE_ALL));
				bindlessDescriptorManager.StoreBuffer(resourceManager.GetBuffer(spotLightsHandle[i]).buffer);
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
		cs_std::math::vec3 cameraPosition = camera.GetComponent<Transform>().GetPosition();
		cs_std::math::mat4 cameraView = camera.GetComponent<Transform>().GetCameraViewMatrix();
		cs_std::math::mat4 cameraViewProjection = camera.GetComponent<PerspectiveCamera>().GetViewProjectionMatrix(cameraView);
		cs_std::graphics::frustum cameraFrustum(cameraViewProjection);

		currentScene.entityManager.ForEach<Behaviours>([&](Behaviours& b) {
			b.OnUpdate(dt);
		});

		std::vector<DirectionalLight::ShaderRepresentation> directionalLights;
		std::vector<PointLight::ShaderRepresentation> pointLights;
		std::vector<SpotLight::ShaderRepresentation> spotLights;

		currentScene.entityManager.ForEach<Transform, DirectionalLight>([&](Transform& transform, DirectionalLight& light) {
			directionalLights.push_back(light.CreateShaderRepresentation(transform));
		});
		currentScene.entityManager.ForEach<Transform, PointLight>([&](Transform& transform, PointLight& light) {
			pointLights.push_back(light.CreateShaderRepresentation(transform));
		});
		currentScene.entityManager.ForEach<Transform, SpotLight>([&](Transform& transform, SpotLight& light) {
			spotLights.push_back(light.CreateShaderRepresentation(transform));
		});

		struct MainPassRenderData
		{
			Vulkan::MeshHandle mesh;
			Vulkan::TextureHandle diffuse;
			Vulkan::TextureHandle normal;
			uint32_t modelID;
			bool doubleSided;
		};

		std::vector<cs_std::math::mat4> meshTransforms;
		std::deque<MainPassRenderData> meshes;
		uint32_t depthPrepassMeshCount = 0;

		currentScene.entityManager.ForEach<Transform, MeshData, Material>([&](Transform& transform, MeshData& mesh, Material& material) {
			// frustum cull
			//if (!cameraFrustum.intersects(mesh.bounds)) return;
			MainPassRenderData data(
				mesh.meshHandle, material.diffuseHandle, material.normalHandle, meshTransforms.size() + 1, material.isDoubleSided
			);
			meshTransforms.push_back(transform.GetModelMatrix());
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

		// Write data to buffers
		resourceManager.GetBuffer(transformsHandle[currentFrameIndex]).buffer.memcpy(&cameraViewProjection, sizeof(cs_std::math::mat4));
		resourceManager.GetBuffer(transformsHandle[currentFrameIndex]).buffer.memcpy(meshTransforms.data(), sizeof(cs_std::math::mat4) * meshTransforms.size(), sizeof(cs_std::math::mat4));
		resourceManager.GetBuffer(directionalLightsHandle[currentFrameIndex]).buffer.memcpy(directionalLights.data(), sizeof(DirectionalLight::ShaderRepresentation) * directionalLights.size());
		resourceManager.GetBuffer(pointLightsHandle[currentFrameIndex]).buffer.memcpy(pointLights.data(), sizeof(PointLight::ShaderRepresentation) * pointLights.size());
		resourceManager.GetBuffer(spotLightsHandle[currentFrameIndex]).buffer.memcpy(spotLights.data(), sizeof(SpotLight::ShaderRepresentation) * spotLights.size());

		cmd.Reset();

		surface.AcquireNextImage(frame.imageAvailable);

		cmd.Begin();

		// Depth prepass
		cmd.DynamicStateSetViewport(depthFramebuffer.GetViewport());
		cmd.DynamicStateSetScissor(depthFramebuffer.GetScissor());

		cmd.BeginRenderPass(depthRenderPass, depthFramebuffer, depthFramebuffer.GetScissor(), Vulkan::Create::DefaultDepthClear());
		cmd.BindPipeline(depthPipeline);
		cmd.BindDescriptorSet(depthPipeline, surface.GetDevice().GetBindlessDescriptorManager().GetSet(), 0, 0, false);

		const struct DepthPrepassParams {
			uint32_t cameraIdx, transformBufferIdx;
		} depthPrepassParams { 0, transformsHandle[currentFrameIndex].GetIndex()};
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
		cmd.BindPipeline(skyboxPipeline);
		cmd.BindDescriptorSet(skyboxPipeline, surface.GetDevice().GetBindlessDescriptorManager().GetSet(), 0, 0, false);

		const struct SkyboxParams {
			uint32_t cameraIdx, transformBufferIdx;
		} skyboxParams{ 0, transformsHandle[currentFrameIndex].GetIndex() };
		const uint32_t skyboxTexture = skyboxTextureHandle.GetIndex() + 1;
		cmd.PushConstants(skyboxPipeline, skyboxParams, VK_SHADER_STAGE_VERTEX_BIT);
		cmd.PushConstants(skyboxPipeline, skyboxTexture, VK_SHADER_STAGE_FRAGMENT_BIT, 2 * sizeof(uint32_t));
		cmd.BindIndexBuffer(resourceManager.GetMesh(skyboxMeshHandle).indexBuffer);
		cmd.BindVertexBuffers({
			*resourceManager.GetMesh(skyboxMeshHandle).GetAttributeBuffer(cs_std::graphics::Attribute::POSITION),
			*resourceManager.GetMesh(skyboxMeshHandle).GetAttributeBuffer(cs_std::graphics::Attribute::TEXCOORD_0),
			}, { 0, 0 });
		cmd.DrawIndexed(resourceManager.GetMesh(skyboxMeshHandle).indexCount, 1, 0, 0, 0);
		cmd.BindDescriptorSet(mainPipeline, surface.GetDevice().GetBindlessDescriptorManager().GetSet(), 0, 0, false);

		// main pass has two push constants, one in the vertex shader, one in the fragment shader
		const struct MainPassVertexParams {
			uint32_t cameraIdx, transformBufferIdx;
		} mainPassParams { 0, transformsHandle[currentFrameIndex].GetIndex()};
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
				data.diffuse.GetIndex() + 1, data.normal.GetIndex() + 1, // first image is the offscreen image
				directionalLightsHandle[currentFrameIndex].GetIndex(), pointLightsHandle[currentFrameIndex].GetIndex(), spotLightsHandle[currentFrameIndex].GetIndex(),
				static_cast<uint32_t>(directionalLights.size()), static_cast<uint32_t>(pointLights.size()), static_cast<uint32_t>(spotLights.size()),
				0, 0,
				cs_std::math::vec4(cameraPosition, 1.0f)
			};
			cmd.PushConstants(mainPipeline, texturesParams, VK_SHADER_STAGE_FRAGMENT_BIT, 2 * sizeof(uint32_t));
			cmd.DrawIndexed(mesh.indexCount, 1, 0, 0, data.modelID);
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
		cmd.BindDescriptorSet(postProcessingPipeline, surface.GetDevice().GetBindlessDescriptorManager().GetSet(), 0, 0, false);

		uint32_t offscreenTexIdx = 0;
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
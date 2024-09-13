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
#define CS_STARTING_TEXT_CHARACTER_COUNT 1024
// HDR	
#define CS_FRAMEBUFFER_COLOR_FORMAT VK_FORMAT_B10G11R11_UFLOAT_PACK32
#define CS_FRAMEBUFFER_DEPTH_FORMAT VK_FORMAT_D32_SFLOAT
#define CS_MSDF_TEXTURE_FORMAT VK_FORMAT_R8G8B8A8_UNORM

std::vector<std::string> GetFonts(const std::string& fontDir)
{
	if (!std::filesystem::exists(fontDir))
	{
		cs_std::console::warn("Could not find font directory: ", fontDir, ". No fonts were found");
		return {};
	}

	if (!std::filesystem::is_directory(fontDir))
	{
		cs_std::console::warn("Provided font directory is not a directory: ", fontDir, ". No fonts were found");
		return {};
	}
	std::vector<std::string> fontPaths;
	for (const auto& entry : std::filesystem::directory_iterator(fontDir))
	{
		if (!entry.is_regular_file()) continue;
		// We only care about true type fonts
		if (entry.path().extension() != ".ttf") continue;
		fontPaths.push_back(entry.path().string());
	}
	return fontPaths;
}
	

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
		// start index of the particle data for this emitter and the number of particles 
		uint32_t startIdx, count;
		Vulkan::TextureHandle texture;
		uint32_t modelID;
	};

	struct TextRenderData
	{
		// reference to the handle that stores character data
		Vulkan::BufferHandle fontCharacterData;
		// start index for the text data, in characters
		uint32_t startIdx, count;
		uint32_t modelID;
		float alignmentOffset;
		float lineOffset;
		Color color;
		uint32_t cameraID;
		float fontSize;
	};

	template<typename T>
	Vulkan::BufferHandle ResizeBufferIfNecessary(Vulkan::ResourceManager& resourceManager, Vulkan::BufferHandle bufferHandle, size_t requiredElementCount)
	{
		Vulkan::Buffer& buffer = resourceManager.GetBuffer(bufferHandle);
		uint32_t bufferElementCount = buffer.buffer.GetElementCount<T>();
		// 1.5x increase
		while (bufferElementCount < requiredElementCount) bufferElementCount += bufferElementCount / 2;
		if (bufferElementCount > buffer.buffer.GetElementCount<T>())
		{
			resourceManager.DestroyBuffer(bufferHandle);
			return resourceManager.CreateBuffer(sizeof(T) * bufferElementCount, VK_SHADER_STAGE_ALL);
		}
		// If resize wasn't necessary, return the original buffer
		return bufferHandle;
	}

	static bool isFirstWindow = true;
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
			.swapchainRecreationCallback = [&](Vulkan::Surface& surface, uint32_t width, uint32_t height, VkPresentModeKHR presentMode)
			{
				cs_std::console::info("Window sized to ", width, "x", height);

				const Vulkan::Vk::Device& device = surface.GetDevice();
				const Vulkan::Vk::Swapchain& swapchain = surface.GetSwapchain();
				const VkSampleCountFlagBits multisamples = static_cast<VkSampleCountFlagBits>(CVar::Get<uint64_t>("rc_multisamples"));
				const double renderScale = CVar::Get<double>("rc_renderscale");

				resourceManager.DestroyTexture(depthImageHandle);
				depthImageHandle = resourceManager.CreateTexture(
					Vulkan::Create::ImageCreateInfo(
						VK_IMAGE_TYPE_2D, CS_FRAMEBUFFER_DEPTH_FORMAT, Vulkan::Create::Extent3D(swapchain.GetExtent3D(), renderScale),
						1, 1, multisamples, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
					),
					Vulkan::Create::AllocationCreateInfo(VMA_MEMORY_USAGE_GPU_ONLY)
				);
				const VkImageView depthImageView = resourceManager.GetTexture(depthImageHandle).image.GetImageView();
				depthFramebuffer = Vulkan::Vk::Framebuffer(surface.GetDevice(), Vulkan::Create::FramebufferCreateInfo(
					depthRenderPass, &depthImageView, Vulkan::Create::Extent2D(swapchain.GetExtent(), renderScale), 1
				));

				resourceManager.DestroyTexture(mainImageHandle);
				mainImageHandle = resourceManager.CreateTexture(
					Vulkan::Create::ImageCreateInfo(
						VK_IMAGE_TYPE_2D, CS_FRAMEBUFFER_COLOR_FORMAT, Vulkan::Create::Extent3D(swapchain.GetExtent3D(), renderScale),
						1, 1, multisamples, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
					),
					Vulkan::Create::AllocationCreateInfo(VMA_MEMORY_USAGE_GPU_ONLY)
				);
				const std::array<VkImageView, 2> attachments2 = { resourceManager.GetTexture(mainImageHandle).image, depthImageView };
				mainFramebuffer = Vulkan::Vk::Framebuffer(device, Vulkan::Create::FramebufferCreateInfo(
					mainRenderPass, attachments2, Vulkan::Create::Extent2D(swapchain.GetExtent(), renderScale), 1
				));
			},
			.presentMode = (CVar::Get<int32_t>("ec_refreshrate") == 0) ? VK_PRESENT_MODE_MAILBOX_KHR : VK_PRESENT_MODE_IMMEDIATE_KHR
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

			const VkSampleCountFlagBits multisamples = static_cast<VkSampleCountFlagBits>(CVar::Get<uint64_t>("rc_multisamples"));

			depthRenderPass = Vulkan::Vk::RenderPass::CreateReversedZDepthRenderPass(device, CS_FRAMEBUFFER_DEPTH_FORMAT, multisamples);
			mainRenderPass = Vulkan::Vk::RenderPass::CreateMainRenderPass(device, CS_FRAMEBUFFER_COLOR_FORMAT, CS_FRAMEBUFFER_DEPTH_FORMAT, multisamples);

			surface.CallRecreationCallback();

			const std::string postProcessingShader = CVar::Get<std::string>("ircs_postprocessing");
			const std::string depthPrepassShader = CVar::Get<std::string>("ircs_depthprepass");
			const std::string mainShader = CVar::Get<std::string>("ircs_main");
			const std::string skyboxShader = CVar::Get<std::string>("ircs_skybox");
			const std::string particleShader = CVar::Get<std::string>("ircs_particle");
			const std::string textShader = CVar::Get<std::string>("ircs_text");

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
			textPipeline = Vulkan::Vk::Pipeline(device, {
				cs_std::binary_file(textShader + ".vert.spv").open().read_if_exists(),
				cs_std::binary_file(textShader + ".frag.spv").open().read_if_exists(),
				Vulkan::Vk::PipelineVariants::GetTextVariant(),
				resourceManager.GetDescriptorSetLayout(), mainRenderPass
			});

			// Skybox mesh
			Construct::Mesh skybox = Construct::SkyboxSphere(32, 32);
			cs_std::graphics::mesh skyboxMesh;
			skyboxMesh.add_attribute(cs_std::graphics::Attribute::POSITION, skybox.vertices);
			skyboxMesh.add_attribute(cs_std::graphics::Attribute::NORMAL, skybox.normals);
			skyboxMesh.add_attribute(cs_std::graphics::Attribute::TEXCOORD_0, skybox.textureUVs);
			skyboxMesh.indices = skybox.indices;

			skyboxMeshHandle = resourceManager.UploadMesh(skyboxMesh);

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

				// Create text advance data buffer
				textAdvanceDataHandle.push_back(resourceManager.CreateBuffer(sizeof(float) * CS_STARTING_TEXT_CHARACTER_COUNT, VK_SHADER_STAGE_ALL));

				// Create text character data buffer (stores the text we want to render)
				// Note this is actually a uint32_t, because it's the smallest type we can use in glsl
				// this means that there is 4 characters in one element
				textCharacterDataHandle.push_back(resourceManager.CreateBuffer(sizeof(char) * CS_STARTING_TEXT_CHARACTER_COUNT, VK_SHADER_STAGE_ALL));
			}

			// Load fonts
			std::vector<std::string> fontPaths = GetFonts((CVar::Exists("pc_fontpath") ? CVar::Get<std::string>("pc_fontpath") : ""));
			if (fontPaths.size() == 0)
				cs_std::console::warn("Could not find any fonts");
			else
			{
				cs_std::console::info("Found ", fontPaths.size(), " fonts on the system");

				for (const auto& fontPath : fontPaths)
				{
					std::string fontName = std::filesystem::path(fontPath).stem().string();
					fonts[fontName] = Font(cs_std::binary_file(fontPath).open().read(), resourceManager);
					cs_std::console::info("Added font ", fontName);
				}
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

		std::vector<float> textAdvanceData;
		std::vector<char> textCharacterData;
		std::vector<TextRenderData> textRenderData;

		// Add active camera matrices to the buffer
		meshTransforms.push_back(cameraView); // Used for billboarding
		meshTransforms.push_back(cameraProjection);  // Used for billboarding
		meshTransforms.push_back(cameraViewProjection);
		meshTransforms.push_back(cs_std::math::ortho( // Used screenspace rendering
			0.0f, -static_cast<float>(swapchain.GetExtent().width),
			-static_cast<float>(swapchain.GetExtent().height), 0.0f,
			-1.0f, 1.0f
		));

		// Update entity behaviours
		currentScene.entityManager.ForEach<Behaviours>([&](Behaviours& b) {
			b.OnUpdate(dt);
		});
		// Update particle emitters
		currentScene.entityManager.ForEach<ParticleEmitter>([&](ParticleEmitter& emitter) {
			emitter.Update(GetTime<float>(), dt);
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

		// Update particle emitters and gather render data
		currentScene.entityManager.ForEach<Transform, ParticleEmitter, ParticleRenderer>([&](Transform& transform, ParticleEmitter& emitter, ParticleRenderer& renderer) {
			meshTransforms.push_back(transform.GetModelMatrix());
			particleEmitters.emplace_back(particles.size(), emitter.liveParticleCount, renderer.texture, meshTransforms.size() - 1);
			for (const ParticleEmitter::Particle& particle : emitter.particles)
			{
				particles.push_back(particle.CreateShaderRepresentation());
			}
		});

		// Gather text to render
		currentScene.entityManager.ForEach<Transform, Text, TextRenderer>([&](Transform& transform, Text& text, TextRenderer& renderer) {
			if (text.text.size() == 0) return;

			// I wish I could handle this edge case better but this works
			bool isRenderable = false;
			for (char c : text.text)
			{
				if (c >= 32 && c <= 126)
				{
					isRenderable = true;
					break;
				}
			}
			if (!isRenderable) return;

			if (fonts.find(text.font) == fonts.end()) return;
			const Font& font = fonts[text.font];

			uint32_t cameraID = (renderer.renderMode == TextRenderer::RenderMode::ScreenSpace) ? 3 : 2;

			// Each line will share the same model matrix
			meshTransforms.push_back(transform.GetModelMatrix());
			uint32_t modelID = meshTransforms.size() - 1;

			uint32_t currentLine = 0;
			uint32_t successiveNewLines = 0;
			uint32_t currentOffset = textCharacterData.size();
			uint32_t count = 0;
			float cumulativeAdvance = 0.0f;
			textAdvanceData.push_back(0.0f);
			bool wasNewLine = false;
			char lastPrintableChar = 0;

			auto calculateAlignmentOffset = [&](char lastChar) -> float
			{
				switch (text.alignment)
				{
					case Text::Alignment::Center: return -(cumulativeAdvance + font.characters[lastChar - 32].advance) / 2.0f;
					case Text::Alignment::Right: return -cumulativeAdvance - font.characters[lastChar - 32].advance;
					default: return 0.0f;
				}
			};

			for (uint32_t i = 0, size = text.text.size(); i < size; i++)
			{
				const char ch = text.text[i];
				if (ch == '\n')
				{
					wasNewLine = true;
					successiveNewLines++;
					continue;
				}
				if (ch == ' ')
				{
					cumulativeAdvance += font.characters[0].advance;
					textAdvanceData.back() = cumulativeAdvance;
					continue;
				}
				if (wasNewLine && count != 0)
				{
					wasNewLine = false;
					textRenderData.emplace_back(
						font.characterDataBufferHandle, currentOffset, count, modelID,
						calculateAlignmentOffset(lastPrintableChar), currentLine * font.lineHeight * text.lineSpacing, text.color, cameraID, text.fontSize
					);
					currentOffset = textCharacterData.size();
					count = 0;
					cumulativeAdvance = 0.0f;
					textAdvanceData.back() = 0.0f;
					currentLine += successiveNewLines;
					successiveNewLines = 0;
				}
				// skip other non-printable characters
				if (ch < 32 || ch > 126) continue;
				cumulativeAdvance += font.characters[ch - 32].advance;
				if (i < size - 1)
					textAdvanceData.push_back(cumulativeAdvance);
				textCharacterData.push_back(ch);
				count++;
				lastPrintableChar = ch;
			}
			textRenderData.emplace_back(
				font.characterDataBufferHandle, currentOffset, count, modelID,
				calculateAlignmentOffset(text.text.back()), currentLine * font.lineHeight * text.lineSpacing, text.color, cameraID, text.fontSize
			);
		});

		cmd.WaitCompletion();

		// Resize buffers if required
		transformsHandle[currentFrameIndex] = ResizeBufferIfNecessary<cs_std::math::mat4>(resourceManager, transformsHandle[currentFrameIndex], meshTransforms.size());
		directionalLightsHandle[currentFrameIndex] = ResizeBufferIfNecessary<DirectionalLight::ShaderRepresentation>(resourceManager, directionalLightsHandle[currentFrameIndex], directionalLights.size());
		pointLightsHandle[currentFrameIndex] = ResizeBufferIfNecessary<PointLight::ShaderRepresentation>(resourceManager, pointLightsHandle[currentFrameIndex], pointLights.size());
		spotLightsHandle[currentFrameIndex] = ResizeBufferIfNecessary<SpotLight::ShaderRepresentation>(resourceManager, spotLightsHandle[currentFrameIndex], spotLights.size());
		particleBufferHandle[currentFrameIndex] = ResizeBufferIfNecessary<ParticleEmitter::ParticleShaderRepresentation>(resourceManager, particleBufferHandle[currentFrameIndex], particles.size());
		textAdvanceDataHandle[currentFrameIndex] = ResizeBufferIfNecessary<float>(resourceManager, textAdvanceDataHandle[currentFrameIndex], textAdvanceData.size());
		textCharacterDataHandle[currentFrameIndex] = ResizeBufferIfNecessary<char>(resourceManager, textCharacterDataHandle[currentFrameIndex], textCharacterData.size());

		// Write data to buffers
		resourceManager.GetBuffer(transformsHandle[currentFrameIndex]).buffer.memcpy(meshTransforms.data(), sizeof(cs_std::math::mat4) * meshTransforms.size());
		resourceManager.GetBuffer(directionalLightsHandle[currentFrameIndex]).buffer.memcpy(directionalLights.data(), sizeof(DirectionalLight::ShaderRepresentation) * directionalLights.size());
		resourceManager.GetBuffer(pointLightsHandle[currentFrameIndex]).buffer.memcpy(pointLights.data(), sizeof(PointLight::ShaderRepresentation) * pointLights.size());
		resourceManager.GetBuffer(spotLightsHandle[currentFrameIndex]).buffer.memcpy(spotLights.data(), sizeof(SpotLight::ShaderRepresentation) * spotLights.size());
		resourceManager.GetBuffer(particleBufferHandle[currentFrameIndex]).buffer.memcpy(particles.data(), sizeof(ParticleEmitter::ParticleShaderRepresentation) * particles.size());
		resourceManager.GetBuffer(textAdvanceDataHandle[currentFrameIndex]).buffer.memcpy(textAdvanceData.data(), sizeof(float) * textAdvanceData.size());
		resourceManager.GetBuffer(textCharacterDataHandle[currentFrameIndex]).buffer.memcpy(textCharacterData.data(), sizeof(char) * textCharacterData.size());

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
		if (activeScene->skybox.IsValid())
		{
			cmd.BindPipeline(skyboxPipeline);
			cmd.BindDescriptorSet(skyboxPipeline, resourceManager.GetDescriptorSet(), 0, 0, false);

			const struct SkyboxParams {
				uint32_t cameraIdx, transformBufferIdx;
			} skyboxParams{ 2, transformsHandle[currentFrameIndex].GetIndex() };
			const uint32_t skyboxTexture = activeScene->skybox.GetIndex();
			cmd.PushConstants(skyboxPipeline, skyboxParams, VK_SHADER_STAGE_VERTEX_BIT);
			cmd.PushConstants(skyboxPipeline, skyboxTexture, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(SkyboxParams));
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
			cmd.PushConstants(mainPipeline, texturesParams, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(MainPassVertexParams));
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
				uint32_t diffuseTexIdx;
			} particleParamsFrag {
				data.texture.GetIndex()
			};
			cmd.PushConstants(particlePipeline, particleParams, VK_SHADER_STAGE_VERTEX_BIT);
			cmd.PushConstants(particlePipeline, particleParamsFrag, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(ParticleVertexParams));
			// One quad for each particle, hence 6 vertices
			cmd.Draw(data.count * 6, 1, 0, data.modelID);
		}

		// Text pass
		cmd.BindDescriptorSet(textPipeline, resourceManager.GetDescriptorSet(), 0, 0, false);
		for (uint32_t i = 0; i < textRenderData.size(); i++)
		{
			TextRenderData& data = textRenderData[i];

			Vulkan::Vk::PipelineVariants variant1 = textPipeline.GetVariants().GetVariant(0);
			Vulkan::Vk::PipelineVariants variant2 = textPipeline.GetVariants().GetVariant(1);

			cmd.BindPipeline(textPipeline[(data.cameraID == 2) ? 0 : 1]);

			const struct TextVertexParams {
				uint32_t cameraIdx, transformBufferIdx, glyphBufferIdx,
						 characterBufferIdx, cumulativeBufferIdx, startingIdx;
				float horizontalOffset, verticalOffset, fontSize;
			} textParamsVertex{
				data.cameraID, transformsHandle[currentFrameIndex].GetIndex(), data.fontCharacterData.GetIndex(),
				textCharacterDataHandle[currentFrameIndex].GetIndex(), textAdvanceDataHandle[currentFrameIndex].GetIndex(), data.startIdx,
				data.alignmentOffset, data.lineOffset, data.fontSize
			};
			const struct TextFragmentParams {
				uint32_t color;
			} textParamsFrag {
				data.color.GetPacked()
			};
			cmd.PushConstants(textPipeline, textParamsVertex, VK_SHADER_STAGE_VERTEX_BIT);
			cmd.PushConstants(textPipeline, textParamsFrag, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(TextVertexParams));
			// One quad for each character, hence 6 vertices
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
			if (allWindowsClosed && !ShouldRestart()) this->Exit();

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
		isFirstWindow = true;
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
			uint32_t refreshRate = 0;

			// -1 for unlimited, 0 for vsync, >0 for fixed refresh rate
			int32_t expectedRefreshRate = CVar::Get<int32_t>("ec_refreshrate");

			if (expectedRefreshRate < 0)
				refreshRate = 0;
			else if (expectedRefreshRate == 0)
				refreshRate = this->windows[0]->GetRefreshRate();
			else
				refreshRate = expectedRefreshRate;

			const double secondsPerFrame = (refreshRate == 0) ? 0.0 : 1.0 / double(refreshRate);
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
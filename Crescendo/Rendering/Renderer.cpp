#include "Renderer.hpp"
#include "Static/Cvar.hpp"
#include "Core/Application.hpp"
#include "Vulkan/Create.hpp"
#include "cs_std/file.hpp"
#include "Libraries/Construct/Construct.hpp"
#include "utils/utils.hpp"
#include "Assets/SPIRVCompiler.hpp"
#include "PushConstantBuffer.hpp"

#define CS_STARTING_OBJECT_COUNT 1024
#define CS_STARTING_DIRECTIONAL_LIGHT_COUNT 8
#define CS_STARTING_POINT_LIGHT_COUNT 8
#define CS_STARTING_SPOT_LIGHT_COUNT 8
#define CS_STARTING_PARTICLE_COUNT 1024
#define CS_STARTING_TEXT_CHARACTER_COUNT 1024
// HDR	
#define CS_FRAMEBUFFER_COLOR_FORMAT VK_FORMAT_B10G11R11_UFLOAT_PACK32
#define CS_FRAMEBUFFER_DEPTH_FORMAT VK_FORMAT_D32_SFLOAT

CS_NAMESPACE_BEGIN
{
	struct MainPassRenderData
	{
		Vulkan::Mesh mesh;
		Material* material;
		uint32_t modelID;
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
		Vulkan::SSBOBufferHandle fontCharacterData;
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
	Vulkan::SSBOBufferHandle ResizeBufferIfNecessary(RenderResourceManager& resourceManager, Vulkan::SSBOBufferHandle bufferHandle, size_t requiredElementCount)
	{
		Vulkan::Vk::Buffer& buffer = resourceManager.GetSSBOBuffer(bufferHandle);
		uint32_t bufferElementCount = buffer.GetElementCount<T>();
		// Extra check so we don't get stuck in a loop
		bufferElementCount = std::max(bufferElementCount, 2u);
		// 1.5x increase
		while (bufferElementCount < requiredElementCount) bufferElementCount += bufferElementCount / 2;
		// If resize wasn't necessary, return the original buffer
		if (bufferElementCount <= buffer.GetElementCount<T>())
			return bufferHandle;
		resourceManager.DestroySSBOBuffer(bufferHandle);
		return resourceManager.CreateSSBOBuffer(sizeof(T) * bufferElementCount);
	}

	void Renderer::Init()
	{
		instance = Vulkan::Instance({
			CVar::Get<bool>("irc_validationlayers"),
			CVar::Get<std::string>("ec_appname"), "Crescendo",
		});
		instance.CreateSurface(Application::Get()->GetWindow()->GetNative(), {
			.swapchainRecreationCallback = [&](Vulkan::Surface& surface, uint32_t width, uint32_t height, VkPresentModeKHR presentMode)
			{
				cs_std::console::info("Window sized to ", width, "x", height);

				const Vulkan::Vk::Device& device = surface.GetDevice();
				const Vulkan::Vk::Swapchain& swapchain = surface.GetSwapchain();
				const VkSampleCountFlagBits multisamples = static_cast<VkSampleCountFlagBits>(CVar::Get<uint64_t>("rc_multisamples"));
				const double renderScale = CVar::Get<double>("rc_renderscale");

				// Depth prepass image
				resourceManager.DestroyTexture(depthImageHandle);
				resourceManager.DestroyFramebuffer(depthFramebufferHandle);
				depthImageHandle = resourceManager.CreateTexture(
					Vulkan::Create::ImageCreateInfo(
						VK_IMAGE_TYPE_2D, CS_FRAMEBUFFER_DEPTH_FORMAT, Vulkan::Create::Extent3D(swapchain.GetExtent3D(), renderScale),
						1, 1, multisamples, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
					),
					Vulkan::Create::AllocationCreateInfo(VMA_MEMORY_USAGE_GPU_ONLY),
					RenderResourceManager::TextureSpecification(
						RenderResourceManager::Colorspace::Linear, RenderResourceManager::Filter::Nearest, RenderResourceManager::Filter::Nearest,
						RenderResourceManager::WrapMode::ClampToEdge, 1.0f, false
					)
				);
				const VkImageView depthImageView = resourceManager.GetTexture(depthImageHandle).GetImageView();
				depthFramebufferHandle = resourceManager.CreateFramebuffer(
					resourceManager.GetRenderPass(depthRenderPassHandle), { depthImageView },  Vulkan::Create::Extent2D(swapchain.GetExtent(), renderScale)
				);

				// Main offscreen image to render to
				resourceManager.DestroyTexture(mainImageHandle);
				resourceManager.DestroyFramebuffer(mainFramebufferHandle);
				mainImageHandle = resourceManager.CreateTexture(
					Vulkan::Create::ImageCreateInfo(
						VK_IMAGE_TYPE_2D, CS_FRAMEBUFFER_COLOR_FORMAT, Vulkan::Create::Extent3D(swapchain.GetExtent3D(), renderScale),
						1, 1, multisamples, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
					),
					Vulkan::Create::AllocationCreateInfo(VMA_MEMORY_USAGE_GPU_ONLY),
					RenderResourceManager::TextureSpecification(
						RenderResourceManager::Colorspace::Linear, RenderResourceManager::Filter::Linear, RenderResourceManager::Filter::Linear,
						RenderResourceManager::WrapMode::ClampToEdge, 1.0f, false
					)
				);
				mainFramebufferHandle = resourceManager.CreateFramebuffer(
					resourceManager.GetRenderPass(mainRenderPassHandle), { resourceManager.GetTexture(mainImageHandle).GetImageView(), depthImageView }, Vulkan::Create::Extent2D(swapchain.GetExtent(), renderScale)
				);

				// Destroy any old bloom buffers
				for (uint32_t i = 0, size = bloomFramebufferHandles.size(); i < size; i++)
				{
					resourceManager.DestroyTexture(bloomImageHandles[i]);
					resourceManager.DestroyFramebuffer(bloomFramebufferHandles[i]);
				}
				bloomFramebufferHandles.clear();
				bloomImageHandles.clear();

				// Create new bloom buffers
				// Bloom levels are calculated based on the width of the window. All the way down to 8x8
				uint32_t bloomLevels = static_cast<uint32_t>(std::max(std::log2(static_cast<float>(width) / 8.0f), 0.0f));
				float currentScale = 0.5f;
				for (uint32_t i = 0; i < bloomLevels; i++, currentScale *= (i == 0) ? 1.0f : 0.5f)
				{
					float bloomImageScale = renderScale * currentScale;

					Vulkan::TextureHandle bloomImageHandle = resourceManager.CreateTexture(
						Vulkan::Create::ImageCreateInfo(
							VK_IMAGE_TYPE_2D, CS_FRAMEBUFFER_COLOR_FORMAT, Vulkan::Create::Extent3D(swapchain.GetExtent3D(), bloomImageScale),
							1, 1, multisamples, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
						),
						Vulkan::Create::AllocationCreateInfo(VMA_MEMORY_USAGE_GPU_ONLY),
						RenderResourceManager::TextureSpecification(
							RenderResourceManager::Colorspace::Linear, RenderResourceManager::Filter::Linear, RenderResourceManager::Filter::Linear,
							RenderResourceManager::WrapMode::ClampToEdge, 1.0f, false
						)
					);
					bloomImageHandles.push_back(bloomImageHandle);
					bloomFramebufferHandles.push_back(resourceManager.CreateFramebuffer(
						resourceManager.GetRenderPass(bloomRenderPassHandle), { resourceManager.GetTexture(bloomImageHandle).GetImageView() }, Vulkan::Create::Extent2D(swapchain.GetExtent(), bloomImageScale)
					));
				}
			},
			.presentMode = (CVar::Get<int32_t>("ec_refreshrate") == 0) ? VK_PRESENT_MODE_MAILBOX_KHR : VK_PRESENT_MODE_IMMEDIATE_KHR
		});
		frameManager = Vulkan::FrameManager(instance.GetSurface(0).GetDevice(), CVar::Get<uint32_t>("rc_framesinflight"));
		resourceManager = RenderResourceManager(
			instance.GetSurface(0).GetDevice(), CVar::Get<uint32_t>("irc_maxbufferdescriptors_bindless"), CVar::Get<uint32_t>("irc_maxtexturedescriptors_bindless")
		);

		{
			Vulkan::Surface& surface = instance.GetSurface(0);
			Vulkan::Vk::Swapchain& swapchain = surface.GetSwapchain();
			Vulkan::Vk::Device& device = surface.GetDevice();

			const VkSampleCountFlagBits multisamples = static_cast<VkSampleCountFlagBits>(CVar::Get<uint64_t>("rc_multisamples"));

			{
				VkAttachmentDescription depthAttachment = Vulkan::Create::AttachmentDescription(
					CS_FRAMEBUFFER_DEPTH_FORMAT, multisamples, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
					VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
				);
				VkAttachmentReference depthAttachmentRef = Vulkan::Create::AttachmentReference(0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
				VkSubpassDescription depthSubpass = Vulkan::Create::SubpassDescription(VK_PIPELINE_BIND_POINT_GRAPHICS, nullptr, nullptr, nullptr, &depthAttachmentRef, nullptr);
				const std::array<VkSubpassDependency, 2> depthDependencies = {
					Vulkan::Create::SubpassDependency(
						VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
						VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_DEPENDENCY_BY_REGION_BIT
					),
					Vulkan::Create::SubpassDependency(
						0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
						VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_DEPENDENCY_BY_REGION_BIT
					)
				};
				depthRenderPassHandle = resourceManager.CreateRenderPass(Vulkan::Create::RenderPassCreateInfo(&depthAttachment, &depthSubpass, depthDependencies));
			}
			{
				VkAttachmentDescription colorAttachment = Vulkan::Create::AttachmentDescription(
					CS_FRAMEBUFFER_COLOR_FORMAT, multisamples, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
					VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
				);
				VkAttachmentReference colorAttachmentRef = Vulkan::Create::AttachmentReference(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
				VkAttachmentDescription depthAttachment = Vulkan::Create::AttachmentDescription(
					CS_FRAMEBUFFER_DEPTH_FORMAT, multisamples, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_DONT_CARE,
					VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
				);
				VkAttachmentReference depthAttachmentRef = Vulkan::Create::AttachmentReference(1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
				const VkSubpassDescription subpass = Vulkan::Create::SubpassDescription(
					VK_PIPELINE_BIND_POINT_GRAPHICS, nullptr, &colorAttachmentRef, nullptr, &depthAttachmentRef, nullptr
				);
				VkSubpassDependency colorDependency = Vulkan::Create::SubpassDependency(
					VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
					VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_DEPENDENCY_BY_REGION_BIT
				);
				VkSubpassDependency colorDependency2 = Vulkan::Create::SubpassDependency(
					0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
					VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_DEPENDENCY_BY_REGION_BIT
				);
				VkSubpassDependency depthDependency = Vulkan::Create::SubpassDependency(
					VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
					VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_DEPENDENCY_BY_REGION_BIT
				);
				std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
				std::array<VkSubpassDependency, 3> dependencies = { colorDependency, colorDependency2, depthDependency };

				mainRenderPassHandle = resourceManager.CreateRenderPass(Vulkan::Create::RenderPassCreateInfo(attachments, &subpass, dependencies));
			}
			{
				VkAttachmentDescription colorAttachment = Vulkan::Create::AttachmentDescription(
					CS_FRAMEBUFFER_COLOR_FORMAT, VK_SAMPLE_COUNT_1_BIT, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
					VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
				);
				VkAttachmentReference colorAttachmentRef = Vulkan::Create::AttachmentReference(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
				const VkSubpassDescription subpass = Vulkan::Create::SubpassDescription(
					VK_PIPELINE_BIND_POINT_GRAPHICS, nullptr, &colorAttachmentRef, nullptr, nullptr, nullptr
				);
				VkSubpassDependency dependency = Vulkan::Create::SubpassDependency(
					VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
					VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_DEPENDENCY_BY_REGION_BIT
				);
				bloomRenderPassHandle = resourceManager.CreateRenderPass(Vulkan::Create::RenderPassCreateInfo(&colorAttachment, &subpass, &dependency));
			}

			surface.CallRecreationCallback();

			const std::string depthPrepassShader = CVar::Get<std::string>("ircs_depthprepass");
			const std::string mainShader = CVar::Get<std::string>("ircs_main");	
			const std::string skyboxShader = CVar::Get<std::string>("ircs_skybox");
			const std::string particleShader = CVar::Get<std::string>("ircs_particle");
			const std::string textShader = CVar::Get<std::string>("ircs_text");
			const std::string postProcessingShader = CVar::Get<std::string>("ircs_postprocessing");

			std::vector<uint8_t> fullScreenQuad = GLSLToSPIRV("./shaders/fullscreen_quad.vert");

			PreprocessorDefines defines;
			defines
				.Define("USE_DIFFUSE_MAP")
				//.Define("USE_METALLIC_MAP")
				//.Define("USE_ROUGHNESS_MAP")
				.Define("USE_NORMAL_MAP")
				//.Define("USE_AO_MAP")
				//.Define("USE_EMISSIVE_MAP")
				.Define("USE_LIGHTING");

			depthPipelineHandle = resourceManager.CreatePipeline({
				GLSLToSPIRV(depthPrepassShader + ".vert"), {},
				Vulkan::Vk::PipelineVariants::GetDepthPrepassVariant(), resourceManager.GetDescriptorSetLayout(), resourceManager.GetRenderPass(depthRenderPassHandle)
			});
			mainPipelineHandle = resourceManager.CreatePipeline({
				GLSLToSPIRV(mainShader + ".vert"), GLSLToSPIRV(mainShader + ".frag", defines),
				Vulkan::Vk::PipelineVariants::GetDefaultVariant(), resourceManager.GetDescriptorSetLayout(), resourceManager.GetRenderPass(mainRenderPassHandle)
			});
			skyboxPipelineHandle = resourceManager.CreatePipeline({
				GLSLToSPIRV(skyboxShader + ".vert"), GLSLToSPIRV(skyboxShader + ".frag"),
				Vulkan::Vk::PipelineVariants::GetSkyboxVariant(), resourceManager.GetDescriptorSetLayout(), resourceManager.GetRenderPass(mainRenderPassHandle)
			});
			particlePipelineHandle = resourceManager.CreatePipeline({
				GLSLToSPIRV(particleShader + ".vert"), GLSLToSPIRV(particleShader + ".frag"),
				Vulkan::Vk::PipelineVariants::GetParticleVariant(), resourceManager.GetDescriptorSetLayout(), resourceManager.GetRenderPass(mainRenderPassHandle)
			});
			textPipelineHandle = resourceManager.CreatePipeline({
				GLSLToSPIRV(textShader + ".vert"), GLSLToSPIRV(textShader + ".frag"),
				Vulkan::Vk::PipelineVariants::GetTextVariant(), resourceManager.GetDescriptorSetLayout(), resourceManager.GetRenderPass(mainRenderPassHandle)
			});
			bloomDownsamplePipelineHandle = resourceManager.CreatePipeline({
				fullScreenQuad, GLSLToSPIRV("./shaders/bloom_downsample.frag"),
				Vulkan::Vk::PipelineVariants::GetPostProcessingVariant(), resourceManager.GetDescriptorSetLayout(), resourceManager.GetRenderPass(bloomRenderPassHandle)
			});
			bloomUpsamplePipelineHandle = resourceManager.CreatePipeline({
				fullScreenQuad, GLSLToSPIRV("./shaders/bloom_upsample.frag"),
				Vulkan::Vk::PipelineVariants::GetPostProcessingVariant(), resourceManager.GetDescriptorSetLayout(), resourceManager.GetRenderPass(bloomRenderPassHandle)
			});
			postProcessingPipelineHandle = resourceManager.CreatePipeline({
				fullScreenQuad, GLSLToSPIRV(postProcessingShader + ".frag"),
				Vulkan::Vk::PipelineVariants::GetPostProcessingVariant(), resourceManager.GetDescriptorSetLayout(), swapchain.GetRenderPass()
			});

			// Skybox mesh
			Construct::Mesh skybox = Construct::SkyboxSphere(32, 32);
			skyboxMesh.vertexAttributes.emplace_back(resourceManager.CreateGPUBuffer(skybox.vertices.data(), sizeof(float) * skybox.vertices.size(), RenderResourceManager::GPUBufferUsage::VertexBuffer), cs_std::graphics::Attribute::POSITION);
			skyboxMesh.vertexAttributes.emplace_back(resourceManager.CreateGPUBuffer(skybox.normals.data(), sizeof(float) * skybox.normals.size(), RenderResourceManager::GPUBufferUsage::VertexBuffer), cs_std::graphics::Attribute::NORMAL);
			skyboxMesh.vertexAttributes.emplace_back(resourceManager.CreateGPUBuffer(skybox.textureUVs.data(), sizeof(float) * skybox.textureUVs.size(), RenderResourceManager::GPUBufferUsage::VertexBuffer), cs_std::graphics::Attribute::TEXCOORD_0);
			skyboxMesh.indexBuffer = resourceManager.CreateGPUBuffer(skybox.indices.data(), sizeof(uint32_t) * skybox.indices.size(), RenderResourceManager::GPUBufferUsage::IndexBuffer);
			skyboxMesh.indexCount = skybox.indices.size();
			skyboxMesh.indexType = VK_INDEX_TYPE_UINT32;

			for (uint32_t i = 0; i < frameManager.GetFrameCount(); i++)
			{
				// Transform buffers
				transformsHandle.push_back(resourceManager.CreateSSBOBuffer(sizeof(cs_std::math::mat4) * CS_STARTING_OBJECT_COUNT));
				// Light buffers
				directionalLightsHandle.push_back(resourceManager.CreateSSBOBuffer(sizeof(DirectionalLight::ShaderRepresentation) * CS_STARTING_DIRECTIONAL_LIGHT_COUNT));
				pointLightsHandle.push_back(resourceManager.CreateSSBOBuffer(sizeof(PointLight::ShaderRepresentation) * CS_STARTING_POINT_LIGHT_COUNT));
				spotLightsHandle.push_back(resourceManager.CreateSSBOBuffer(sizeof(SpotLight::ShaderRepresentation) * CS_STARTING_SPOT_LIGHT_COUNT));
				// Particle buffers
				particleBufferHandle.push_back(resourceManager.CreateSSBOBuffer(sizeof(ParticleEmitter::ParticleShaderRepresentation) * CS_STARTING_PARTICLE_COUNT));
				// Text buffers
				textAdvanceDataHandle.push_back(resourceManager.CreateSSBOBuffer(sizeof(float) * CS_STARTING_TEXT_CHARACTER_COUNT));
				textCharacterDataHandle.push_back(resourceManager.CreateSSBOBuffer(sizeof(char) * CS_STARTING_TEXT_CHARACTER_COUNT));
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
	void Renderer::RenderScene(Scene& scene)
	{
		// Commonly used resources
		Vulkan::Surface& surface = instance.GetSurface(0);
		Vulkan::Vk::Swapchain& swapchain = surface.GetSwapchain();
		Vulkan::FrameResources& frame = frameManager.GetCurrentFrame();
		uint32_t currentFrameIndex = frameManager.GetCurrentFrameIndex();
		Vulkan::Vk::GraphicsCommandQueue& cmd = frame.commandBuffer;

		// Active camera in the scene
		Entity camera = scene.entityManager.GetEntity(scene.activeCamera);

		cs_std::math::vec3 cameraPosition = camera.GetComponent<Transform>().GetPosition();
		cs_std::math::mat4 cameraView = camera.GetComponent<Transform>().GetCameraViewMatrix();
		cs_std::math::mat4 cameraProjection = camera.GetComponent<PerspectiveCamera>().GetProjectionMatrix(Application::Get()->GetWindow()->GetAspectRatio());
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
			0.0f, static_cast<float>(swapchain.GetExtent().width),
			static_cast<float>(swapchain.GetExtent().height), 0.0f,
			-1.0f, 1.0f
		));

		// Add lights to the buffer
		scene.entityManager.ForEach<Transform, DirectionalLight>([&](Transform& transform, DirectionalLight& light) {
			if (light.intensity == 0.0f)
				return;
			directionalLights.push_back(light.CreateShaderRepresentation(transform));
		});
		scene.entityManager.ForEach<Transform, PointLight>([&](Transform& transform, PointLight& light) {
			if (light.intensity == 0.0f)
				return;
			pointLights.push_back(light.CreateShaderRepresentation(transform));
		});
		scene.entityManager.ForEach<Transform, SpotLight>([&](Transform& transform, SpotLight& light) {
			if (light.intensity == 0.0f)
				return;
			spotLights.push_back(light.CreateShaderRepresentation(transform));
		});

		scene.entityManager.ForEach<Transform, MeshData, Material>([&](Transform& transform, MeshData& mesh, Material& material) {
			meshTransforms.push_back(transform.GetModelMatrix());
			MainPassRenderData data(
				mesh.meshHandle, &material, meshTransforms.size() - 1
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
			float depthA = cs_std::math::distance2(cameraPosition, cs_std::math::vec3(meshTransforms[a.modelID][3]));
			float depthB = cs_std::math::distance2(cameraPosition, cs_std::math::vec3(meshTransforms[b.modelID][3]));
			return depthA > depthB;
		});

		// Update particle emitters and gather render data
		scene.entityManager.ForEach<Transform, ParticleEmitter, ParticleRenderer>([&](Transform& transform, ParticleEmitter& emitter, ParticleRenderer& renderer) {
			meshTransforms.push_back(transform.GetModelMatrix());
			particleEmitters.emplace_back(particles.size(), emitter.liveParticleCount, renderer.texture, meshTransforms.size() - 1);
			for (uint32_t i = 0; i < emitter.liveParticleCount; i++)
			{
				const ParticleEmitter::Particle& particle = emitter.particles[i];
				particles.push_back(particle.CreateShaderRepresentation());
			}
		});

		// Gather text to render
		scene.entityManager.ForEach<Transform, Text, TextRenderer>([&](Transform& transform, Text& text, TextRenderer& renderer) {
			if (text.text.size() == 0)
				return;

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
			if (!isRenderable)
				return;

			if (fonts.find(text.font) == fonts.end())
				return;
			const Font& font = fonts[text.font];

			uint32_t cameraID = (renderer.renderMode == TextRenderer::RenderMode::ScreenSpace) ? 3 : 2;

			// Each line will share the same model matrix
			meshTransforms.push_back(transform.GetModelMatrix());
			uint32_t modelID = meshTransforms.size() - 1;

			uint32_t currentLine = (renderer.renderMode == TextRenderer::RenderMode::ScreenSpace) ? 1 : 0;
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
					case Text::Alignment::Center:
						return -(cumulativeAdvance + font.characters[lastChar - 32].advance) / 2.0f;
					case Text::Alignment::Right:
						return -cumulativeAdvance - font.characters[lastChar - 32].advance;
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
				if (ch < 32 || ch > 126)
					continue;
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
		resourceManager.GetSSBOBuffer(transformsHandle[currentFrameIndex]).memcpy(meshTransforms.data(), sizeof(cs_std::math::mat4) * meshTransforms.size());
		resourceManager.GetSSBOBuffer(directionalLightsHandle[currentFrameIndex]).memcpy(directionalLights.data(), sizeof(DirectionalLight::ShaderRepresentation) * directionalLights.size());
		resourceManager.GetSSBOBuffer(pointLightsHandle[currentFrameIndex]).memcpy(pointLights.data(), sizeof(PointLight::ShaderRepresentation) * pointLights.size());
		resourceManager.GetSSBOBuffer(spotLightsHandle[currentFrameIndex]).memcpy(spotLights.data(), sizeof(SpotLight::ShaderRepresentation) * spotLights.size());
		resourceManager.GetSSBOBuffer(particleBufferHandle[currentFrameIndex]).memcpy(particles.data(), sizeof(ParticleEmitter::ParticleShaderRepresentation) * particles.size());
		resourceManager.GetSSBOBuffer(textAdvanceDataHandle[currentFrameIndex]).memcpy(textAdvanceData.data(), sizeof(float) * textAdvanceData.size());
		resourceManager.GetSSBOBuffer(textCharacterDataHandle[currentFrameIndex]).memcpy(textCharacterData.data(), sizeof(char) * textCharacterData.size());

		cmd.Reset();

		surface.AcquireNextImage(frame.imageAvailable);

		cmd.Begin();

		// Depth prepass

		Vulkan::Vk::Framebuffer& depthFramebuffer = resourceManager.GetFramebuffer(depthFramebufferHandle);
		Vulkan::Vk::RenderPass& depthRenderPass = resourceManager.GetRenderPass(depthRenderPassHandle);
		Vulkan::Vk::Pipeline& depthPipeline = resourceManager.GetPipeline(depthPipelineHandle);

		PushConstantBuffer depthPrepassParams;
		depthPrepassParams
			.Push(2)
			.Push(transformsHandle[currentFrameIndex].GetIndex());

		cmd.DynamicStateSetViewport(depthFramebuffer.GetViewport());
		cmd.DynamicStateSetScissor(depthFramebuffer.GetScissor());
		cmd.BeginRenderPass(depthRenderPass, depthFramebuffer, depthFramebuffer.GetScissor(), Vulkan::Create::DefaultDepthClear());
		cmd.BindPipeline(depthPipeline);
		cmd.BindDescriptorSet(depthPipeline, resourceManager.GetDescriptorSet(), 0, 0, false);
		cmd.PushConstants(depthPipeline, depthPrepassParams.Get(), depthPrepassParams.GetSize(), VK_SHADER_STAGE_VERTEX_BIT);
		for (size_t i = 0; i < depthPrepassMeshCount; i++)
		{
			MainPassRenderData& data = meshes[i];
			cmd.BindPipeline(depthPipeline[data.material->isDoubleSided]);
			cmd.BindIndexBuffer(resourceManager.GetGPUBuffer(data.mesh.indexBuffer));
			cmd.BindVertexBuffers({ resourceManager.GetGPUBuffer(data.mesh.GetAttributeBufferHandle(cs_std::graphics::Attribute::POSITION)) }, { 0 });
			cmd.DrawIndexed(data.mesh.indexCount, 1, 0, 0, data.modelID);
		}
		cmd.EndRenderPass();

		// Main pass

		Vulkan::Vk::Framebuffer& mainFramebuffer = resourceManager.GetFramebuffer(mainFramebufferHandle);
		Vulkan::Vk::RenderPass& mainRenderPass = resourceManager.GetRenderPass(mainRenderPassHandle);
		Vulkan::Vk::Pipeline& mainPipeline = resourceManager.GetPipeline(mainPipelineHandle);
		Vulkan::Vk::Pipeline& skyboxPipeline = resourceManager.GetPipeline(skyboxPipelineHandle);
		Vulkan::Vk::Pipeline& particlePipeline = resourceManager.GetPipeline(particlePipelineHandle);
		Vulkan::Vk::Pipeline& textPipeline = resourceManager.GetPipeline(textPipelineHandle);

		cmd.DynamicStateSetViewport(mainFramebuffer.GetViewport());
		cmd.DynamicStateSetScissor(mainFramebuffer.GetScissor());
		cmd.BeginRenderPass(mainRenderPass, mainFramebuffer, mainFramebuffer.GetScissor(), { Vulkan::Create::ClearValue(0.0f, 0.0f, 0.0f, 1.0f) });

		// Render skybox
		if (scene.skybox.IsValid())
		{
			PushConstantBuffer skyboxParams;
			skyboxParams
				// Vertex
				.Push(2)
				.Push(transformsHandle[currentFrameIndex].GetIndex())
				// Fragment
				.Push(scene.skybox.GetIndex());

			cmd.BindPipeline(skyboxPipeline);
			cmd.BindDescriptorSet(skyboxPipeline, resourceManager.GetDescriptorSet(), 0, 0, false);
			cmd.PushConstants(skyboxPipeline, skyboxParams.Get(), skyboxParams.GetSize() - sizeof(uint32_t), VK_SHADER_STAGE_VERTEX_BIT);
			cmd.PushConstants(skyboxPipeline, skyboxParams.Get(2 * sizeof(uint32_t)), sizeof(uint32_t), VK_SHADER_STAGE_FRAGMENT_BIT, 2 * sizeof(uint32_t));
			cmd.BindIndexBuffer(resourceManager.GetGPUBuffer(skyboxMesh.indexBuffer));
			cmd.BindVertexBuffers({
				resourceManager.GetGPUBuffer(skyboxMesh.GetAttributeBufferHandle(cs_std::graphics::Attribute::POSITION)),
				resourceManager.GetGPUBuffer(skyboxMesh.GetAttributeBufferHandle(cs_std::graphics::Attribute::TEXCOORD_0))
			}, { 0, 0 });
			cmd.DrawIndexed(skyboxMesh.indexCount, 1, 0, 0, 0);
		}

		// main pass has two push constants, one in the vertex shader, one in the fragment shader
		PushConstantBuffer mainPassParams;
		mainPassParams
			// Vertex
			.Push(2)
			.Push(transformsHandle[currentFrameIndex].GetIndex())
			// Fragment
			.Separate()
			.Push(directionalLightsHandle[currentFrameIndex].GetIndex())
			.Push(pointLightsHandle[currentFrameIndex].GetIndex())
			.Push(spotLightsHandle[currentFrameIndex].GetIndex())
			.Push(static_cast<uint32_t>(directionalLights.size()))
			.Push(static_cast<uint32_t>(pointLights.size()))
			.Push(static_cast<uint32_t>(spotLights.size()))
			.Push(cs_std::math::vec4(cameraPosition, 1.0f));

		cmd.BindDescriptorSet(mainPipeline, resourceManager.GetDescriptorSet(), 0, 0, false);
		cmd.PushConstants(mainPipeline, mainPassParams.Get(), 2 * sizeof(uint32_t), VK_SHADER_STAGE_VERTEX_BIT);
		cmd.PushConstants(mainPipeline, mainPassParams.Get(2 * sizeof(uint32_t)), mainPassParams.GetSize(2 * sizeof(uint32_t)), VK_SHADER_STAGE_FRAGMENT_BIT, 2 * sizeof(uint32_t));
		// Render objects, renders opaque first, then transparent
		for (size_t i = 0; i < meshes.size(); i++)
		{
			MainPassRenderData& data = meshes[i];
			PushConstantBuffer mainPassFragmentParams;
			data.material->BuildPushConstantBuffer(mainPassFragmentParams);

			cmd.BindPipeline(mainPipeline[data.material->isDoubleSided]);
			cmd.BindIndexBuffer(resourceManager.GetGPUBuffer(data.mesh.indexBuffer));
			cmd.BindVertexBuffers({
				resourceManager.GetGPUBuffer(data.mesh.GetAttributeBufferHandle(cs_std::graphics::Attribute::POSITION)),
				resourceManager.GetGPUBuffer(data.mesh.GetAttributeBufferHandle(cs_std::graphics::Attribute::NORMAL)),
				resourceManager.GetGPUBuffer(data.mesh.GetAttributeBufferHandle(cs_std::graphics::Attribute::TANGENT)),
				resourceManager.GetGPUBuffer(data.mesh.GetAttributeBufferHandle(cs_std::graphics::Attribute::TEXCOORD_0))
			}, { 0, 0, 0, 0 });
			cmd.PushConstants(mainPipeline, mainPassFragmentParams.Get(), mainPassFragmentParams.GetSize(), VK_SHADER_STAGE_FRAGMENT_BIT, mainPassParams.GetSize());
			cmd.DrawIndexed(data.mesh.indexCount, 1, 0, 0, data.modelID);
		}

		// Particle pass
		cmd.BindPipeline(particlePipeline);
		cmd.BindDescriptorSet(particlePipeline, resourceManager.GetDescriptorSet(), 0, 0, false);
		for (uint32_t i = 0; i < particleEmitters.size(); i++)
		{
			ParticleEmitterRenderData& data = particleEmitters[i];
			PushConstantBuffer particleParams;
			particleParams
				// Vertex
				.Push(2)
				.Push(transformsHandle[currentFrameIndex].GetIndex())
				.Push(particleBufferHandle[currentFrameIndex].GetIndex())
				.Push(data.startIdx)
				.Push(Application::Get()->GetTime<float>())
				// Fragment
				.Push(data.texture.GetIndex());

			cmd.PushConstants(particlePipeline, particleParams.Get(), particleParams.GetSize() - sizeof(float), VK_SHADER_STAGE_VERTEX_BIT);
			cmd.PushConstants(particlePipeline, particleParams.Get(5 * sizeof(float)), sizeof(float), VK_SHADER_STAGE_FRAGMENT_BIT, 5 * sizeof(float));
			// One quad for each particle, hence 6 vertices
			cmd.Draw(data.count * 6, 1, 0, data.modelID);
		}

		// Text pass
		cmd.BindDescriptorSet(textPipeline, resourceManager.GetDescriptorSet(), 0, 0, false);
		for (uint32_t i = 0; i < textRenderData.size(); i++)
		{
			TextRenderData& data = textRenderData[i];
			PushConstantBuffer textParams;
			textParams
				// Vertex
				.Push(data.cameraID)
				.Push(transformsHandle[currentFrameIndex].GetIndex())
				.Push(data.fontCharacterData.GetIndex())
				.Push(textCharacterDataHandle[currentFrameIndex].GetIndex())
				.Push(textAdvanceDataHandle[currentFrameIndex].GetIndex())
				.Push(data.startIdx)
				.Push(data.alignmentOffset)
				.Push(data.lineOffset)
				.Push(data.fontSize)
				// Fragment
				.Push(data.color.GetPacked());

			cmd.BindPipeline(textPipeline[(data.cameraID == 2) ? 0 : 1]);
			cmd.PushConstants(textPipeline, textParams.Get(), textParams.GetSize() - sizeof(float), VK_SHADER_STAGE_VERTEX_BIT);
			cmd.PushConstants(textPipeline, textParams.Get(9 * sizeof(float)), sizeof(float), VK_SHADER_STAGE_FRAGMENT_BIT, 9 * sizeof(float));
			// One quad for each character, hence 6 vertices
			cmd.Draw(data.count * 6, 1, 0, data.modelID);
		}

		cmd.EndRenderPass();

		// Bloom downsample

		Vulkan::Vk::Pipeline& bloomDownsamplePipeline = resourceManager.GetPipeline(bloomDownsamplePipelineHandle);
		Vulkan::Vk::RenderPass& bloomRenderPass = resourceManager.GetRenderPass(bloomRenderPassHandle);
		for (uint32_t i = 1; i < bloomFramebufferHandles.size(); i++)
		{
			Vulkan::Vk::Framebuffer& bloomFramebuffer = resourceManager.GetFramebuffer(bloomFramebufferHandles[i]);
			PushConstantBuffer bloomParams;
			bloomParams
				.Push((i == 1) ? mainImageHandle.GetIndex() : bloomImageHandles[i - 1].GetIndex()) // Sample the previous bloom buffer
				.Push(1.0f); // Threshold

			cmd.DynamicStateSetViewport(bloomFramebuffer.GetViewport());
			cmd.DynamicStateSetScissor(bloomFramebuffer.GetScissor());
			cmd.BeginRenderPass(bloomRenderPass, bloomFramebuffer, bloomFramebuffer.GetScissor(), { Vulkan::Create::ClearValue(0.0f, 0.0f, 0.0f, 1.0f) });
			cmd.BindPipeline(bloomDownsamplePipeline);
			cmd.BindDescriptorSet(bloomDownsamplePipeline, resourceManager.GetDescriptorSet(), 0, 0, false);
			cmd.PushConstants(bloomDownsamplePipeline, bloomParams.Get(), bloomParams.GetSize(), VK_SHADER_STAGE_FRAGMENT_BIT);
			cmd.Draw(6);
			cmd.EndRenderPass();
		}

		// Bloom upsample
		Vulkan::Vk::Pipeline& bloomUpsamplePipeline = resourceManager.GetPipeline(bloomUpsamplePipelineHandle);
		for (int32_t i = static_cast<int32_t>(bloomFramebufferHandles.size()) - 1; i > 0; i--)
		{
			Vulkan::Vk::Framebuffer& bloomFramebuffer = resourceManager.GetFramebuffer(bloomFramebufferHandles[i - 1]);
			PushConstantBuffer bloomParams;
			bloomParams.Push(bloomImageHandles[i].GetIndex());

			cmd.DynamicStateSetViewport(bloomFramebuffer.GetViewport());
			cmd.DynamicStateSetScissor(bloomFramebuffer.GetScissor());
			cmd.BeginRenderPass(bloomRenderPass, bloomFramebuffer, bloomFramebuffer.GetScissor(), { Vulkan::Create::ClearValue(0.0f, 0.0f, 0.0f, 1.0f) });
			cmd.BindPipeline(bloomUpsamplePipeline);
			cmd.BindDescriptorSet(bloomUpsamplePipeline, resourceManager.GetDescriptorSet(), 0, 0, false);
			cmd.PushConstants(bloomUpsamplePipeline, bloomParams.Get(), bloomParams.GetSize(), VK_SHADER_STAGE_FRAGMENT_BIT);
			cmd.Draw(6);
			cmd.EndRenderPass();
		}

		// Final post processing pass
		Vulkan::Vk::Pipeline& postProcessingPipeline = resourceManager.GetPipeline(postProcessingPipelineHandle);
		PushConstantBuffer postProcessingParams;
		postProcessingParams
			.Push(mainImageHandle.GetIndex()) // Offscreen texture
			.Push(bloomImageHandles[0].GetIndex()); // Bloom texture

		cmd.DynamicStateSetViewport(swapchain.GetViewport(true));
		cmd.DynamicStateSetScissor(swapchain.GetScissor());
		cmd.BeginRenderPass(
			swapchain.GetRenderPass(), swapchain.GetFramebuffer(swapchain.GetAcquiredImageIndex()).framebuffer, swapchain.GetScissor(),
			{ Vulkan::Create::ClearValue(0.0f, 0.0f, 0.0f, 0.0f), Vulkan::Create::DefaultDepthClear() }
		);
		cmd.BindPipeline(postProcessingPipeline);
		cmd.BindDescriptorSet(postProcessingPipeline, resourceManager.GetDescriptorSet(), 0, 0, false);
		cmd.PushConstants(postProcessingPipeline, postProcessingParams.Get(), postProcessingParams.GetSize(), VK_SHADER_STAGE_FRAGMENT_BIT);
		cmd.Draw(6);
		cmd.EndRenderPass();

		cmd.End();
		cmd.Submit(frame.imageAvailable, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, frame.renderFinished);

		surface.Present(cmd.GetQueue(), frame.renderFinished);

		frameManager.AdvanceFrame();
	}
	void Renderer::WaitIdle()
	{
		for (size_t i = 0, size = instance.GetSurfaceCount(); i < size; i++)
			instance.GetSurface(i).GetDevice().WaitIdle();
	}
}
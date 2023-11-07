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
	glm::mat4 transform;
	cs_std::graphics::bounding_aabb bounds;
	uint32_t textureID, normalID;
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

	/// <summary>
	/// Describes gpu mesh metadata
	/// </summary>
	struct Mesh
	{
		struct Attribute
		{
			Crescendo::Vulkan::Buffer buffer;
			uint32_t elements;
			cs_std::graphics::Attribute attribute;
		};
		Crescendo::Vulkan::Buffer indexBuffer;
		uint32_t indexCount;
		std::vector<Attribute> vertexAttributes;
	};

	struct Texture
	{
		Crescendo::Vulkan::Image image;
		VkDescriptorSet set;
	};

	int frame = 0;
	double lastTime = 0.0;
	
	// Fixed setup
	Crescendo::Vulkan::Instance instance;
	Crescendo::Vulkan::Device device;
	Crescendo::Vulkan::TransferCommandQueue transferQueue;

	// Variable setup
	int frameIndex = 0;
	std::vector<Crescendo::Vulkan::Frame> frameData;
	Crescendo::Vulkan::Swapchain swapchain;

	// Resources
	cs_std::packed_vector<Crescendo::Vulkan::RenderPass> renderPasses;
	cs_std::packed_vector<Crescendo::Vulkan::Framebuffer> framebuffers;
	cs_std::packed_vector<Crescendo::Vulkan::Pipelines> pipelines;
	cs_std::packed_vector<Mesh> meshes;
	cs_std::packed_vector<Texture> textures;
	std::vector<VkSampler> samplers;

	Crescendo::Vulkan::Image depthBuffer;
	Texture shadowMap;
	VkSampler shadowMapSampler;
	Crescendo::Vulkan::Framebuffer shadowMapFramebuffer;
public:
	void CreateSwapchain()
	{
		constexpr VkFormat DEFAULT_DEPTH_FORMAT = VK_FORMAT_D32_SFLOAT;
		constexpr VkFormat DEFAULT_SHADOW_FORMAT = VK_FORMAT_D16_UNORM;

		this->device.WaitIdle();
		// Get glfw window size
		int width = 0, height = 0;
		while (width == 0 || height == 0)
		{
			glfwGetFramebufferSize(static_cast<GLFWwindow*>(this->instance.GetWindow()), &width, &height);
			glfwWaitEvents();
		}

		this->framebuffers.clear();
		this->renderPasses.clear();
		// Explicitly destroy swapchain and depth buffer
		this->swapchain.~Swapchain();
		this->depthBuffer.~Image();

		this->swapchain = this->instance.CreateSwapchain(this->device, VK_PRESENT_MODE_MAILBOX_KHR, VkExtent2D(CVar::Get<int64_t>("ec_windowwidth"), CVar::Get<int64_t>("ec_windowheight")));
		
		// Create renderpasses
		uint32_t drpIdx = this->renderPasses.insert(this->device.CreateDefaultRenderPass(this->swapchain.GetImageFormat(), DEFAULT_DEPTH_FORMAT));
		uint32_t smrpIdx = this->renderPasses.insert(this->device.CreateDefaultShadowRenderPass(DEFAULT_SHADOW_FORMAT));
		
		// Create images
		this->depthBuffer = this->device.CreateImage(Crescendo::Vulkan::Create::ImageCreateInfo(
			VK_IMAGE_TYPE_2D, DEFAULT_DEPTH_FORMAT, this->swapchain.GetExtent3D(), 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
		), VMA_MEMORY_USAGE_GPU_ONLY);

		this->shadowMap.image = this->device.CreateImage(Crescendo::Vulkan::Create::ImageCreateInfo(
			VK_IMAGE_TYPE_2D, DEFAULT_SHADOW_FORMAT, Crescendo::Vulkan::Create::Extent3D(4096, 4096, 1), 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
		), VMA_MEMORY_USAGE_GPU_ONLY);

		// Create the frame buffers
		std::vector<VkImageView> attachments { nullptr, this->depthBuffer.imageView };
		for (const auto& swapChainImage : this->swapchain)
		{
			attachments[0] = swapChainImage.imageView;
			this->framebuffers.insert(this->device.CreateFramebuffer(this->renderPasses[drpIdx], attachments, this->swapchain.GetExtent(), true, true));
		}

		// Create shadow map frame buffer
		this->shadowMapFramebuffer = this->device.CreateFramebuffer(this->renderPasses[smrpIdx], { this->shadowMap.image.imageView }, { 4096, 4096 }, false, true);

		this->shadowMapSampler = this->device.CreateSampler(Crescendo::Vulkan::Create::SamplerCreateInfo(
			VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
			1.0f, 1.0f, VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE
		));
		this->shadowMap.set = this->device.AllocateDescriptorSet(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, this->device.GetFragmentSamplerLayout());
		VkDescriptorImageInfo imageInfo = Crescendo::Vulkan::Create::DescriptorImageInfo(
			this->shadowMapSampler, this->shadowMap.image.imageView, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
		);
		VkWriteDescriptorSet write = Crescendo::Vulkan::Create::WriteDescriptorSet(
			this->shadowMap.set, 0, 0, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &imageInfo, nullptr, nullptr
		);
		vkUpdateDescriptorSets(this->device, 1, &write, 0, nullptr);
	}
	Mesh UploadMesh(const cs_std::graphics::mesh& mesh)
	{
		/* ---------------------------------------------------------------- 0 - Mesh validation ---------------------------------------------------------------- */

		constexpr size_t ELEMENTS_PER_ATTRIBUTE[static_cast<size_t>(cs_std::graphics::Attribute::ATTRIBUTE_COUNT)]{
			3, // POSITION
			3, // NORMAL
			4, // TANGENT
			2, // TEXCOORD0
			2, // TEXCOORD1
			4, // COLOR0
			4, // JOINTS0
			4  // WEIGHTS0
		};
		for (const auto& attribute : mesh.attributes) CS_ASSERT(attribute.data.size() % ELEMENTS_PER_ATTRIBUTE[static_cast<size_t>(attribute.attribute)] == 0, "Invalid mesh data! mesh has " + std::to_string(attribute.data.size()) + " elements! but expected a multiple of " + std::to_string(ELEMENTS_PER_ATTRIBUTE[static_cast<size_t>(attribute.attribute)]) + "!");
		CS_ASSERT(mesh.indices.size() % 3 == 0, "Invalid mesh data! mesh has " + std::to_string(mesh.indices.size()) + " indices! but expected a multiple of 3!");
		// TODO assert for potential buffer overflows

		/* ---------------------------------------------------------------- 1 - Create GPU buffers ---------------------------------------------------------------- */

		Mesh gpuMesh = {};
		gpuMesh.indexCount = static_cast<uint32_t>(mesh.indices.size());
		gpuMesh.indexBuffer = this->device.CreateBuffer(
			sizeof(uint32_t) * mesh.indices.size(),
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VMA_MEMORY_USAGE_GPU_ONLY
		);
		for (const auto& attribute : mesh.attributes)
		{
			gpuMesh.vertexAttributes.emplace_back(
				this->device.CreateBuffer(
					sizeof(float) * attribute.data.size(),
					VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
					VMA_MEMORY_USAGE_GPU_ONLY
				), static_cast<uint32_t>(attribute.data.size()), attribute.attribute
			);
		}

		/* ---------------------------------------------------------------- 2 - Staging ---------------------------------------------------------------- */

		// Determine the buffer offsets
		std::vector<uint32_t> bufferOffsets(1, 0);
		bufferOffsets.push_back(bufferOffsets.back() + mesh.indices.size() * sizeof(uint32_t));
		for (const auto& attribute : mesh.attributes) bufferOffsets.push_back(bufferOffsets.back() + attribute.data.size() * sizeof(float));

		// Stage all data into one buffer, reduces allocations
		Crescendo::Vulkan::Buffer staging = this->device.CreateBuffer(bufferOffsets.back(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
		staging.Fill(0, mesh.indices.data(), mesh.indices.size() * sizeof(uint32_t));
		for (size_t i = 0; i < mesh.attributes.size(); i++) staging.Fill(bufferOffsets[i + 1], mesh.attributes[i].data.data(), mesh.attributes[i].data.size() * sizeof(float));
	
		/* ---------------------------------------------------------------- 2 - Transfer ---------------------------------------------------------------- */

		// Upload data to the GPU
		this->transferQueue.InstantSubmit([&](const Crescendo::Vulkan::TransferCommandQueue& cmd) {
			// Copy index data
			VkBufferCopy copy = Crescendo::Vulkan::Create::BufferCopy(0, 0, mesh.indices.size() * sizeof(uint32_t));
			cmd.CopyBuffer(staging.buffer, gpuMesh.indexBuffer, copy);

			// Copy other vertex attributes
			uint32_t offset = 1;
			for (const auto& attribute : gpuMesh.vertexAttributes)
			{
				VkBufferCopy copy = Crescendo::Vulkan::Create::BufferCopy(bufferOffsets[offset], 0, attribute.elements * sizeof(float));
				cmd.CopyBuffer(staging.buffer, attribute.buffer, copy);
				offset++;
			}
		});

		return gpuMesh;
	}
	Texture UploadTexture(const cs_std::image& image, bool generateMipmaps)
	{
		// For now every image uses the same format. This can be very wasteful, at least until variable formats are implemented
		// Or we have texture compression
		constexpr VkFormat DEFAULT_FORMAT = VK_FORMAT_R8G8B8A8_SRGB;

		const size_t imageSize = static_cast<size_t>(image.width) * static_cast<size_t>(image.height) * static_cast<size_t>(image.channels);
		const uint8_t mipLevels = 1 + generateMipmaps ? static_cast<uint8_t>(std::log2(std::max(image.width, image.height))) : 0;

		// Create samplers dynamically as required for mip levels
		// TODO add anisotropy
		VkSamplerCreateInfo samplerInfo = Crescendo::Vulkan::Create::SamplerCreateInfo(
			VK_FILTER_LINEAR, VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT,
			1.0f, 1.0f
		);
		for (uint8_t i = static_cast<uint8_t>(samplers.size()); i < mipLevels; i++)
		{
			samplerInfo.maxLod = static_cast<float>(i);
			this->samplers.push_back(this->device.CreateSampler(samplerInfo));
		}

		// Stage the image data
		Crescendo::Vulkan::Buffer staging = this->device.CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY).Fill(0, image.data.data(), imageSize);
	
		// Create the image
		const VkExtent3D extent = Crescendo::Vulkan::Create::Extent3D(image.width, image.height, 1);
		Crescendo::Vulkan::Image texture = this->device.CreateImage(Crescendo::Vulkan::Create::ImageCreateInfo(
			VK_IMAGE_TYPE_2D, DEFAULT_FORMAT, extent,
			mipLevels, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_SHARING_MODE_EXCLUSIVE, 0, nullptr, VK_IMAGE_LAYOUT_UNDEFINED
		), VMA_MEMORY_USAGE_GPU_ONLY);

		this->transferQueue.InstantSubmit([&](const Crescendo::Vulkan::TransferCommandQueue& cmd) {
			// Transition to blit compatible
			VkImageMemoryBarrier barrier = Crescendo::Vulkan::Create::ImageMemoryBarrier(
				0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, texture.image,
				Crescendo::Vulkan::Create::ImageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT, 0, mipLevels, 0, 1)
			);
			cmd.ResourceBarrier(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, barrier);
			// Copy buffer to image
			const VkBufferImageCopy region = Crescendo::Vulkan::Create::BufferImageCopy(
				0, 0, 0, Crescendo::Vulkan::Create::ImageSubresourceLayers(VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1), Crescendo::Vulkan::Create::Offset3D(0, 0, 0), extent
			);
			cmd.CopyBufferToImage(staging.buffer, texture.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, region);
			// Transition the image to a transfer destination
			barrier = Crescendo::Vulkan::Create::ImageMemoryBarrier(
				0, 0, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_UNDEFINED,
				VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, texture.image,
				Crescendo::Vulkan::Create::ImageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1)
			);
			// Generate each of the mips
			uint32_t mipWidth = image.width, mipHeight = image.height;
			for (uint32_t i = 1; i < mipLevels; i++)
			{
				barrier.subresourceRange.baseMipLevel = i - 1;
				barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
				cmd.ResourceBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, barrier);
				const VkImageBlit blit = Crescendo::Vulkan::Create::ImageBlit(
					Crescendo::Vulkan::Create::ImageSubresourceLayers(VK_IMAGE_ASPECT_COLOR_BIT, i - 1, 0, 1),
					Crescendo::Vulkan::Create::Offset3D(0, 0, 0), Crescendo::Vulkan::Create::Offset3D(static_cast<int32_t>(mipWidth), static_cast<int32_t>(mipHeight), 1),
					Crescendo::Vulkan::Create::ImageSubresourceLayers(VK_IMAGE_ASPECT_COLOR_BIT, i, 0, 1),
					Crescendo::Vulkan::Create::Offset3D(0, 0, 0), Crescendo::Vulkan::Create::Offset3D(static_cast<int32_t>(std::max(mipWidth / 2, 1u)), static_cast<int32_t>(std::max(mipHeight / 2, 1u)), 1)
				);
				cmd.BlitImage(texture.image, texture.image, blit);
				barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
				cmd.ResourceBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, barrier);
				mipWidth = std::max(mipWidth / 2, 1u);
				mipHeight = std::max(mipHeight / 2, 1u);
			}
			barrier.subresourceRange.baseMipLevel = mipLevels - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			cmd.ResourceBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, barrier);
		});

		VkDescriptorSet descriptorSet = this->device.AllocateDescriptorSet(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, this->device.GetFragmentSamplerLayout());
		VkDescriptorImageInfo imageInfo = Crescendo::Vulkan::Create::DescriptorImageInfo(this->samplers[mipLevels - 1], texture.imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		VkWriteDescriptorSet write = Crescendo::Vulkan::Create::WriteDescriptorSet(descriptorSet, 0, 0, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &imageInfo, nullptr, nullptr);
		this->device.WriteDescriptorSet(write);

		return Texture(std::move(texture), descriptorSet);
	}
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
			glm::vec4(-25.0f, 25.0f, -25.0f, 25.0f),
			glm::vec2(-250.0f, 250.0f)
		);

		/* ---------------------------------------------------------------- 0 - Vulkan setup ---------------------------------------------------------------- */

		// Instance setup
		this->instance = Crescendo::Vulkan::Instance(
			CVar::Get<bool>("rc_validationlayers"),
			CVar::Get<std::string>("rc_appname"),
			CVar::Get<std::string>("rc_enginename"),
			this->GetWindow()->GetNative()
		);
		this->device = this->instance.CreateDevice(CVar::Get<int64_t>("rc_descriptorsetsperpool"));
		this->transferQueue = this->device.CreateTransferCommandQueue();

		for (uint32_t i = 0, fif = CVar::Get<int64_t>("rc_framesinflight"); i < fif; i++)
			this->frameData.emplace_back(this->device.CreateGraphicsCommandQueue(), this->device.CreateSemaphore(), this->device.CreateSemaphore());
		this->CreateSwapchain();

		/* ---------------------------------------------------------------- 1.0 - Shader data ---------------------------------------------------------------- */
		struct Shader { std::string name; Crescendo::Vulkan::PipelineVariants variants; };

		// Generates 4, pipelines for
		// Non-Transparent, one-sided
		// Transparent, one-sided
		// Non-Transparent, two-sided
		// Transparent, two-sided
		const Crescendo::Vulkan::PipelineVariants defaultVariant = Crescendo::Vulkan::PipelineVariants(
			this->renderPasses[0],
			Crescendo::Vulkan::PipelineVariants::FillMode::Solid,
			Crescendo::Vulkan::PipelineVariants::CullMode::Back | Crescendo::Vulkan::PipelineVariants::CullMode::None,
			Crescendo::Vulkan::PipelineVariants::Multisamples::One,
			Crescendo::Vulkan::PipelineVariants::DepthFunc::Less,
			Crescendo::Vulkan::PipelineVariants::DepthTest::Enabled,
			Crescendo::Vulkan::PipelineVariants::DepthWrite::Enabled | Crescendo::Vulkan::PipelineVariants::DepthWrite::Disabled
		);

		std::vector<Shader> shaderList {
			{ "./shaders/compiled/mesh", defaultVariant },
			{ "./shaders/compiled/mesh-unlit", defaultVariant},
			{ "./shaders/compiled/skybox", Crescendo::Vulkan::PipelineVariants(
				this->renderPasses[0],
				Crescendo::Vulkan::PipelineVariants::FillMode::Solid,
				Crescendo::Vulkan::PipelineVariants::CullMode::Back,
				Crescendo::Vulkan::PipelineVariants::Multisamples::One,
				Crescendo::Vulkan::PipelineVariants::DepthFunc::Less,
				Crescendo::Vulkan::PipelineVariants::DepthTest::Disabled,
				Crescendo::Vulkan::PipelineVariants::DepthWrite::Disabled
			)  },
			{ "./shaders/compiled/shadow_map", Crescendo::Vulkan::PipelineVariants(
				this->renderPasses[1],
				Crescendo::Vulkan::PipelineVariants::FillMode::Solid,
				Crescendo::Vulkan::PipelineVariants::CullMode::None,
				Crescendo::Vulkan::PipelineVariants::Multisamples::One,
				Crescendo::Vulkan::PipelineVariants::DepthFunc::Less,
				Crescendo::Vulkan::PipelineVariants::DepthTest::Enabled,
				Crescendo::Vulkan::PipelineVariants::DepthWrite::Enabled
			) },
			{ "./shaders/compiled/ui", Crescendo::Vulkan::PipelineVariants(
				this->renderPasses[0],
				Crescendo::Vulkan::PipelineVariants::FillMode::Solid,
				Crescendo::Vulkan::PipelineVariants::CullMode::None,
				Crescendo::Vulkan::PipelineVariants::Multisamples::One,
				Crescendo::Vulkan::PipelineVariants::DepthFunc::Less,
				Crescendo::Vulkan::PipelineVariants::DepthTest::Enabled,
				Crescendo::Vulkan::PipelineVariants::DepthWrite::Disabled
			) }
		};
		for (const auto& shader : shaderList)
		{
			this->pipelines.insert(this->device.CreatePipelines(
				cs_std::binary_file(shader.name + ".vert.spv").open().read(),
				cs_std::binary_file(shader.name + ".frag.spv").open().read(),
				shader.variants
			));
		}

		/* ---------------------------------------------------------------- 1.1 - Descriptor Data ---------------------------------------------------------------- */

		// Lit meshes
		// Creates 1 set for all bindings, no need to specify the bindings anymore
		uint32_t litMesh0 = this->pipelines[0].CreateDescriptorSets(0, 3); // One per frame in flight (fif)
		uint32_t litMesh1 = this->pipelines[0].CreateDescriptorSet(1);
		// Unlit meshes
		uint32_t unlitMesh0 = this->pipelines[1].CreateDescriptorSets(0, 3); // One per fif
		// Skybox
		uint32_t skybox0 = this->pipelines[2].CreateDescriptorSets(0, 3); // One per fif
		// Shadow map
		uint32_t shadowMap0 = this->pipelines[3].CreateDescriptorSets(0, 3); // One per fif
		// UI
		uint32_t ui0 = this->pipelines[4].CreateDescriptorSets(0, 3); // One per fif

		this->pipelines[0].UpdateDescriptorData(litMesh1, 1, 0, glm::vec3(0.3f, 0.4f, 0.3f));

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
			quadModel.meshes.push_back(quadMesh);
			quadModel.meshAttributes.push_back(quadAttributes);
		}

		std::vector<cs_std::graphics::model> models =
		{
			//IO::LoadGLTF("./assets/tristan/TRISTANSEXY.gltf"),
			IO::LoadGLTF("./assets/modern-sponza/modern-sponza.gltf"),
			IO::LoadGLTF("./assets/sponza-curtains/sponza-curtains.gltf"),
			//IO::LoadGLTF("./assets/sponza-ivy/sponza-ivy.gltf"),
			//IO::LoadOBJ("./assets/obj-sponza/sponza.obj"),
			//IO::LoadGLTF("./assets/companion-cube/scene.gltf"),
			IO::LoadGLTF("./assets/tree/tree.gltf"),
			//IO::LoadGLTF("./assets/chair/chair.gltf"),
			skyboxModel,
			quadModel
		};

		int textureIndex = 0;
		std::map<std::filesystem::path, uint32_t> seenTextures;

		for (auto& model : models)
		{
			for (uint32_t i = 0; i < model.meshes.size(); i++)
			{
				auto& mesh = model.meshes[i];
				auto& attributes = model.meshAttributes[i];

				if (!mesh.has_attribute(cs_std::graphics::Attribute::TANGENT)) cs_std::graphics::generate_tangents(mesh);

				this->meshes.insert(this->UploadMesh(mesh));

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
					attributes.transform,
					cs_std::graphics::bounding_aabb(mesh.get_attribute(cs_std::graphics::Attribute::POSITION).data).transform(attributes.transform),
					seenTextures[attributes.diffuse], seenTextures[attributes.normal],
					attributes.isTransparent, attributes.isDoubleSided, true
				);
			}
		}

		seenTextures.erase("");
		std::vector<std::filesystem::path> textureStrings(seenTextures.size());
		for (const auto& texture : seenTextures) textureStrings[texture.second] = texture.first;

		std::vector<cs_std::image> images(textureStrings.size());

		std::atomic<uint32_t> finishedTasks = 0;
		uint32_t last = 0;

		for (uint32_t i = 0; i < textureStrings.size(); i++)
		{
			this->taskQueue.push_back([&images, &textureStrings, i, &finishedTasks]() { images[i] = IO::LoadImage(textureStrings[i]); finishedTasks++; });
		}
		
		while (!this->taskQueue.finished())
		{
			uint32_t local = finishedTasks;
			for (uint32_t i = 0; i < local - last; i++) cs_std::console::raw("#");
			last = local;
		}
		cs_std::console::raw('\n');
		this->taskQueue.sleep();

		 for (auto& image : images) this->textures.insert(this->UploadTexture(image, false));

		/* ---------------------------------------------------------------- 1.3 - Texture Data ---------------------------------------------------------------- */

		cs_std::console::log("All systems go!");

		//std::map<std::filesystem::path, uint32_t> seenTexturesDiffuse, seenTexturesNormal;
		//this->meshCount = 0;
		//uint32_t i = 0, j = 0;
		//double accumulatingTime = 0.0;
		//for (auto& model : models)
		//{
		//	this->meshCount += model.meshes.size();
		//	
		//	for (auto& mesh : model.meshes)
		//	{
		//		if (!mesh.normal.empty() && seenTexturesNormal.find(mesh.normal) == seenTexturesNormal.end())
		//		{
		//			seenTexturesNormal[mesh.normal] = j;
		//			j++;
		//		}

		//		this->modelData.emplace_back(
		//			cs_std::graphics::bounding_aabb(mesh.meshData.get_attribute(cs_std::graphics::Attribute::POSITION).data).transform(mesh.transform),
		//			seenTexturesDiffuse[mesh.diffuse], seenTexturesNormal[mesh.normal],
		//			mesh.isTransparent, mesh.isDoubleSided, true
		//		);

		//		Entity entity = EntityManager::CreateEntity();
		//		entity.AddComponent<Transform>(Transform(mesh.transform));
		//	
		//		this->entities.push_back(entity);
		//	}
		//}
	}
	void OnUpdate(double dt)
	{

		/* ---------------------------------------------------------------- Game update ---------------------------------------------------------------- */

		this->camera.Update();

		float currentTime = this->GetTime<float>() / 2.0f;
		this->shadowMapCamera.SetPosition(glm::vec3(std::sinf(currentTime) * 100.0f, std::cosf(currentTime) * 100.0f, 0.0f));
		this->shadowMapCamera.LookAt(glm::vec3(0.0f, 0.0f, 0.0f));

		/* ---------------------------------------------------------------- Render preparation ---------------------------------------------------------------- */

		// Render prep
		const glm::mat4 projections[2] = { this->camera.camera.GetViewProjectionMatrix(), this->shadowMapCamera.GetViewProjectionMatrix() };
		const glm::vec4 lightingPositions[2] = { glm::vec4(this->shadowMapCamera.GetPosition(), 1.0f), glm::vec4(this->camera.camera.GetPosition(), 1.0f) };

		// Arguments in order: set index, set, binding, data
		this->pipelines[0].UpdateDescriptorData(frameIndex, 0, 0, projections);
		this->pipelines[0].UpdateDescriptorData(frameIndex, 0, 1, lightingPositions);
		this->pipelines[1].UpdateDescriptorData(frameIndex, 0, 0, projections[0]);
		this->pipelines[2].UpdateDescriptorData(frameIndex, 0, 0, projections[0]);
		this->pipelines[3].UpdateDescriptorData(frameIndex, 0, 0, projections[1]);
		this->pipelines[4].UpdateDescriptorData(frameIndex, 0, 0, this->UICamera.GetViewProjectionMatrix());

		/* ---------------------------------------------------------------- Render commands ---------------------------------------------------------------- */

		Crescendo::Vulkan::Frame& cur = this->frameData[this->frameIndex];
		Crescendo::Vulkan::GraphicsCommandQueue& cmd = cur.cmd;

		cmd.WaitCompletion();
		cmd.Reset();

		uint32_t currentImage = this->swapchain.AcquireNextImage(cur.presentReady);
		if (this->swapchain.NeedsRecreation())
		{
			this->CreateSwapchain();
			currentImage = this->swapchain.AcquireNextImage(cur.presentReady);
		}
		const Crescendo::Vulkan::Framebuffer& framebuffer = this->framebuffers[currentImage];

		const uint32_t usePipeline = 0;
		const uint32_t skyboxPipeline = 2;
		const uint32_t shadowPipeline = 3;

		cmd.Begin();
			// Shadowmap pass
			cmd.DynamicStateSetViewport(this->shadowMapFramebuffer.GetViewport());
			cmd.DynamicStateSetScissor(this->shadowMapFramebuffer.GetScissor());
			cmd.BeginRenderPass(this->shadowMapFramebuffer.renderPass, this->shadowMapFramebuffer, this->shadowMapFramebuffer.GetScissor(), { Crescendo::Vulkan::Create::DefaultDepthClear() });
			cs_std::graphics::frustum frustum(this->shadowMapCamera.GetViewProjectionMatrix());
			// Bind pipeline
			cmd.BindPipeline(this->pipelines[shadowPipeline][0]);
			// Bind data descriptor sets
			cmd.BindDescriptorSets(this->pipelines[shadowPipeline], { this->pipelines[shadowPipeline].descriptorSets[0][frameIndex].set }, { 0 });
			for (uint32_t i = 0; i < this->meshes.capacity() - 2; i++)
			{
				// If outside the frustum, skip
				if (!frustum.intersects(this->modelData[i].bounds)) continue;
				// Push the constants
				cmd.PushConstants(this->pipelines[shadowPipeline], this->modelData[i].transform, VK_SHADER_STAGE_VERTEX_BIT);
				// Bind vertex meshes
				std::vector<VkBuffer> buffers;
				for (uint32_t cpvaf = 0, mvaf = 0; cpvaf < this->pipelines[shadowPipeline].vertexAttributes.size(); mvaf++)
				{
					if (this->pipelines[shadowPipeline].vertexAttributes[cpvaf] == this->meshes[i].vertexAttributes[mvaf].attribute)
					{
						buffers.push_back(this->meshes[i].vertexAttributes[mvaf].buffer);
						cpvaf++;
					}
				}
				CS_ASSERT(buffers.size() == this->pipelines[shadowPipeline].vertexAttributes.size(), "Not all vertex attributes are bound!");
				const std::vector<VkDeviceSize> bufferOffsets(buffers.size(), 0);
				cmd.BindVertexBuffers(buffers, bufferOffsets);
				cmd.BindIndexBuffer(this->meshes[i].indexBuffer);
				cmd.DrawIndexed(this->meshes[i].indexCount, 1, 0, 0, 0);
			}
			cmd.EndRenderPass();
			// Normal pass
			cmd.DynamicStateSetViewport(framebuffer.GetViewport(true));
			cmd.DynamicStateSetScissor(framebuffer.GetScissor());
			cmd.BeginRenderPass(framebuffer.renderPass, framebuffer, framebuffer.GetScissor(), { { 0.0f, 0.0f, 0.0f, 1.0f }, Crescendo::Vulkan::Create::DefaultDepthClear() });
				// Skybox			
				{
					cmd.BindPipeline(this->pipelines[skyboxPipeline][0]);
					cmd.PushConstants(this->pipelines[skyboxPipeline], glm::translate(glm::mat4(1.0f), this->camera.camera.GetPosition()), VK_SHADER_STAGE_VERTEX_BIT);
					cmd.BindDescriptorSets(this->pipelines[skyboxPipeline], { this->pipelines[skyboxPipeline].descriptorSets[0][frameIndex].set }, { 0 });
					cmd.BindDescriptorSet(this->pipelines[skyboxPipeline], this->textures[this->modelData[this->meshes.capacity() - 2].textureID].set, 0, 1);
					// Bind vertex meshes
					std::vector<VkBuffer> buffers;
					for (uint32_t cpvaf = 0, mvaf = 0; cpvaf < this->pipelines[2].vertexAttributes.size(); mvaf++)
					{
						if (this->pipelines[skyboxPipeline].vertexAttributes[cpvaf] == this->meshes[this->meshes.capacity() - 2].vertexAttributes[mvaf].attribute)
						{
							buffers.push_back(this->meshes[this->meshes.capacity() - 2].vertexAttributes[mvaf].buffer);
							cpvaf++;
						}
					}
					CS_ASSERT(buffers.size() == this->pipelines[skyboxPipeline].vertexAttributes.size(), "Not all vertex attributes are bound!");
					const std::vector<VkDeviceSize> bufferOffsets(buffers.size(), 0);
					cmd.BindVertexBuffers(buffers, bufferOffsets);
					cmd.BindIndexBuffer(this->meshes[this->meshes.capacity() - 2].indexBuffer);
					cmd.DrawIndexed(this->meshes[this->meshes.capacity() - 2].indexCount, 1, 0, 0, 0);
				}
				frustum = cs_std::graphics::frustum(this->camera.camera.GetViewProjectionMatrix());
				for (uint32_t i = 0; i < this->meshes.capacity() - 2; i++)
				{
					// If outside the frustum, skip
					if (!frustum.intersects(this->modelData[i].bounds)) continue;
					// Select the correct pipeline
					if (!this->modelData[i].isDoubleSided && !this->modelData[i].isTransparent) cmd.BindPipeline(this->pipelines[usePipeline][0]);
					if (!this->modelData[i].isDoubleSided &&  this->modelData[i].isTransparent) cmd.BindPipeline(this->pipelines[usePipeline][1]);
					if ( this->modelData[i].isDoubleSided && !this->modelData[i].isTransparent) cmd.BindPipeline(this->pipelines[usePipeline][2]);
					if ( this->modelData[i].isDoubleSided &&  this->modelData[i].isTransparent) cmd.BindPipeline(this->pipelines[usePipeline][3]);
					// Push the constants
					cmd.PushConstants(this->pipelines[usePipeline], this->modelData[i].transform, VK_SHADER_STAGE_VERTEX_BIT);
					// Bind data descriptor sets
					cmd.BindDescriptorSets(this->pipelines[usePipeline], { this->pipelines[usePipeline].descriptorSets[0][frameIndex].set, this->pipelines[usePipeline].descriptorSets[1][0].set }, { 0, 0, 0 });
					// Bind texture descriptor sets
					cmd.BindDescriptorSets(this->pipelines[usePipeline], { this->textures[this->modelData[i].textureID].set, this->textures[this->modelData[i].normalID].set, this->shadowMap.set }, {}, 2);
					// Bind vertex meshes
					std::vector<VkBuffer> buffers;
					for (uint32_t cpvaf = 0, mvaf = 0; cpvaf < this->pipelines[usePipeline].vertexAttributes.size(); mvaf++)
					{
						if (this->pipelines[usePipeline].vertexAttributes[cpvaf] == this->meshes[i].vertexAttributes[mvaf].attribute)
						{
							buffers.push_back(this->meshes[i].vertexAttributes[mvaf].buffer);
							cpvaf++;
						}
					}
					CS_ASSERT(buffers.size() == this->pipelines[usePipeline].vertexAttributes.size(), "Not all vertex attributes are bound!");
					const std::vector<VkDeviceSize> bufferOffsets(buffers.size(), 0);
					cmd.BindVertexBuffers(buffers, bufferOffsets);
					cmd.BindIndexBuffer(this->meshes[i].indexBuffer);
					cmd.DrawIndexed(this->meshes[i].indexCount, 1, 0, 0, 0);
				}
			cmd.EndRenderPass();
		cmd.End();
		cmd.Submit(cur.presentReady, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, cur.renderFinish);

		/* ---------------------------------------------------------------- Post-render and present ---------------------------------------------------------------- */

		cmd.Present(this->swapchain, currentImage, cur.renderFinish);

		this->frameIndex = (this->frameIndex + 1) % this->frameData.size();

		//	// Anything UI related
		//	this->renderer.renderer.CmdBindPipeline(2 * 4 + 2);

		//	glm::mat4 model = glm::mat4(1.0f);
		//	model = glm::translate(model, glm::vec3(200.0f, -200.0f, 0.0f));
		//	model = glm::scale(model, glm::vec3(400.0f, 400.0f, 1.0f));

		//	this->renderer.renderer.CmdUpdatePushConstant(Renderer::ShaderStage::Vertex, model);
		//	this->renderer.renderer.CmdBindTexture(1, Renderer::SHADOW_MAP_ID);

		//	this->renderer.renderer.CmdDraw(this->meshCount - 1);
		//	actualDrawCount++;
		// 
		// 
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
	}
	void OnExit()
	{
		this->device.WaitIdle();
		// Destroy samplers
		for (auto& sampler : this->samplers) vkDestroySampler(this->device, sampler, nullptr);
		vkDestroySampler(this->device, this->shadowMapSampler, nullptr);
	}
};

CrescendoRegisterApp(Sandbox);
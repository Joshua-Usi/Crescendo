#include "ResourceManager.hpp"
#include "Create.hpp"

CS_NAMESPACE_BEGIN::Vulkan
{
	constexpr uint32_t UNIFORM_BINDING = 0, BUFFER_BINDING = 1, IMAGE_BINDING = 2;
	template<typename index_type>
	MeshHandle TemplateUploadMesh(const cs_std::graphics::mesh<index_type>&mesh, Vk::Device* device, const Vk::TransferCommandQueue & transferQueue, cs_std::slotmap<Mesh>&meshes)
	{
		// Typical number of elements per attribute, Only includes GLTF required attributes
		constexpr size_t ELEMENTS_PER_ATTRIBUTE[static_cast<size_t>(cs_std::graphics::Attribute::ATTRIBUTE_COUNT)] {
			3, 3, 4, // POSITION, NORMAL, TANGENT
			2, 2, 4,  // TEXCOORD0, TEXCOORD1, COLOR0
			4, 4  // JOINTS0, WEIGHTS0
		};

		/* ---------------------------------------------------------------- 0. - Data validation ---------------------------------------------------------------- */
		CS_ASSERT_WARNING(mesh.attributes.size() > 0, "Invalid mesh data. Mesh must have at least one or more attributes");
		CS_ASSERT_WARNING(mesh.has_attribute(cs_std::graphics::Attribute::POSITION), "Invalid mesh data. Mesh must have a position attribute");
		for (const auto& attribute : mesh.attributes)
		{
			CS_ASSERT_WARNING(attribute.data.size() % ELEMENTS_PER_ATTRIBUTE[static_cast<size_t>(attribute.attribute)] == 0, "Invalid mesh data. Mesh has " + std::to_string(attribute.data.size()) + " elements which is not a multiple of " + std::to_string(ELEMENTS_PER_ATTRIBUTE[static_cast<size_t>(attribute.attribute)]));
		}
		CS_ASSERT_WARNING(mesh.indices.size() > 0, "Invalid mesh data. Mesh must have at least one or more indices");
		CS_ASSERT_WARNING(mesh.indices.size() % 3 == 0, "Invalid mesh data. Mesh has " + std::to_string(mesh.indices.size()) + " indices which is not a multiple of 3");

		/* ---------------------------------------------------------------- 1. GPU Buffer creation ---------------------------------------------------------------- */
		Mesh gpuMesh;
		gpuMesh.indexCount = static_cast<uint32_t>(mesh.indices.size());
		gpuMesh.indexBuffer = Vk::Buffer(
			device->GetAllocator(),
			Create::BufferCreateInfo(0, gpuMesh.indexCount * sizeof(index_type), VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_SHARING_MODE_EXCLUSIVE, nullptr),
			Create::AllocationCreateInfo(VMA_MEMORY_USAGE_CPU_TO_GPU)
		);
		for (const auto& attribute : mesh.attributes)
		{
			gpuMesh.vertexAttributes.emplace_back(
				Vk::Buffer(
					device->GetAllocator(),
					Create::BufferCreateInfo(0, attribute.data.size() * sizeof(float), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_SHARING_MODE_EXCLUSIVE, nullptr),
					Create::AllocationCreateInfo(VMA_MEMORY_USAGE_CPU_TO_GPU)
				),
				static_cast<uint32_t>(attribute.data.size()),
				attribute.attribute
			);
		}

		/* ---------------------------------------------------------------- 2. Staging ---------------------------------------------------------------- */
		std::vector<uint32_t> bufferOffsets(1, 0);
		for (const auto& attribute : mesh.attributes) bufferOffsets.push_back(bufferOffsets.back() + attribute.data.size() * sizeof(float));

		// Since indices can be 16 or 32 bit, we use a separate buffer to prevent alignment issues
		Vk::Buffer indexStaging(
			device->GetAllocator(),
			Create::BufferCreateInfo(0, mesh.indices.size() * sizeof(index_type), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, nullptr),
			Create::AllocationCreateInfo(VMA_MEMORY_USAGE_CPU_ONLY)
		);
		indexStaging.memcpy(mesh.indices.data(), mesh.indices.size() * sizeof(index_type), 0);

		Vk::Buffer staging(
			device->GetAllocator(),
			Create::BufferCreateInfo(0, bufferOffsets.back(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, nullptr),
			Create::AllocationCreateInfo(VMA_MEMORY_USAGE_CPU_ONLY)
		);
		for (uint32_t i = 0; i < mesh.attributes.size(); i++) staging.memcpy(mesh.attributes[i].data.data(), mesh.attributes[i].data.size() * sizeof(float), bufferOffsets[i]);

		/* ---------------------------------------------------------------- 3. - Transfer ---------------------------------------------------------------- */
		transferQueue.InstantSubmit([&](const Vk::TransferCommandQueue& cmd) {
			cmd.CopyBuffer(indexStaging, gpuMesh.indexBuffer, Create::BufferCopy(0, 0, mesh.indices.size() * sizeof(index_type)));
			for (uint32_t i = 0; i < gpuMesh.vertexAttributes.size(); i++)
			{
				cmd.CopyBuffer(staging, gpuMesh.vertexAttributes[i].buffer, Create::BufferCopy(bufferOffsets[i], 0, gpuMesh.vertexAttributes[i].elements * sizeof(float)));
			}
		});

		/* ---------------------------------------------------------------- 4. - Handle Creation ---------------------------------------------------------------- */
		return MeshHandle(meshes.emplace(std::move(gpuMesh)));
	}
	VkFormat GetImageFormat(ResourceManager::Colorspace colorSpace, uint16_t channels)
	{
		switch (colorSpace)
		{
			case ResourceManager::Colorspace::SRGB:
				switch (channels)
				{
					case 1: return VK_FORMAT_R8_SRGB;
					case 2: return VK_FORMAT_R8G8_SRGB;
					case 3: return VK_FORMAT_R8G8B8_SRGB;
					case 4: return VK_FORMAT_R8G8B8A8_SRGB;
				}
				[[fallthrough]];
			case ResourceManager::Colorspace::Linear:
				switch (channels)
				{
					case 1: return VK_FORMAT_R8_UNORM;
					case 2: return VK_FORMAT_R8G8_UNORM;
					case 3: return VK_FORMAT_R8G8B8_UNORM;
					case 4: return VK_FORMAT_R8G8B8A8_UNORM;
				}
		}
		return VK_FORMAT_UNDEFINED;
	}
	ResourceManager::ResourceManager() : device(nullptr), transferQueue() {}
	ResourceManager::ResourceManager(Vk::Device& device, const ResourceManagerSpecification& resourceManagerSpec)
		: device(&device), transferQueue(device, device.GetTransferQueue(), false)
	{
		constexpr std::array<VkDescriptorType, 3> types{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
		// For simplicity, we treat buffers and uniforms as the same thing, so the true maximum is 2x the maxBuffers
		const std::array<uint32_t, 3> setsSizes = { { resourceManagerSpec.maxBuffers, resourceManagerSpec.maxBuffers, resourceManagerSpec.maxImages } };
		std::array<VkDescriptorSetLayoutBinding, 3> bindings = {};
		std::array<VkDescriptorBindingFlags, 3> flags = {};
		std::array<VkDescriptorPoolSize, 3> sizes = {};
		for (uint8_t i = 0; i < types.size(); i++)
		{
			bindings[i] = Create::DescriptorSetLayoutBinding(i, types[i], setsSizes[i], VK_SHADER_STAGE_ALL);
			flags[i] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
			sizes[i] = { types[i], setsSizes[i] };
		}
		const VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlags = Create::DescriptorSetLayoutBindingFlagsCreateInfo(flags);
		const VkDescriptorSetLayoutCreateInfo layoutCreateInfo = Create::DescriptorSetLayoutCreateInfo(&bindingFlags, VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT, bindings);
		vkCreateDescriptorSetLayout(device, &layoutCreateInfo, nullptr, &layout);
		// Create the pool
		const VkDescriptorPoolCreateInfo poolInfo = Create::DescriptorPoolCreateInfo(VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT, 1, sizes);
		vkCreateDescriptorPool(device, &poolInfo, nullptr, &pool);
		// Create the global set
		const VkDescriptorSetAllocateInfo allocateInfo = Create::DescriptorSetAllocateInfo(pool, &layout);
		vkAllocateDescriptorSets(device, &allocateInfo, &set);
	}
	ResourceManager::~ResourceManager()
	{
		if (device == nullptr) return;
		vkDestroyDescriptorSetLayout(*device, layout, nullptr);
		vkDestroyDescriptorPool(*device, pool, nullptr);
	}
	ResourceManager::ResourceManager(ResourceManager&& other) noexcept
		: device(other.device), transferQueue(std::move(transferQueue)),
		buffers(std::move(other.buffers)), textures(std::move(other.textures)), meshes(std::move(other.meshes)),
		samplers(std::move(other.samplers)), layout(other.layout), pool(other.pool), set(other.set) {}
	ResourceManager& ResourceManager::operator=(ResourceManager&& other) noexcept
	{
		device = other.device;
		transferQueue = std::move(other.transferQueue);
		buffers = std::move(other.buffers);
		textures = std::move(other.textures);
		meshes = std::move(other.meshes);
		samplers = std::move(other.samplers);
		layout = other.layout; other.layout = nullptr;
		pool = other.pool; other.pool = nullptr;
		set = other.set; other.set = nullptr;
		return *this;
	
}
	BufferHandle ResourceManager::CreateBuffer(VkDeviceSize size, VkShaderStageFlags shaderStage)
	{
		CS_ASSERT(size > 0, "Buffer size must be greater than 0");

		VkBufferCreateInfo bufferCreateInfo = Create::BufferCreateInfo(0, size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE, nullptr);
		VmaAllocationCreateInfo vmaAllocInfo{};
		vmaAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

		BufferHandle bufferHandle(buffers.emplace(Vk::Buffer(device->GetAllocator(), bufferCreateInfo, vmaAllocInfo)));
		Vulkan::Buffer* bufferPtr = buffers.get(bufferHandle);

		CS_ASSERT(bufferPtr != nullptr, "Failed to add buffer to slotmap");

		const VkDescriptorBufferInfo descriptorBufferInfo = Create::DescriptorBufferInfo(bufferPtr->buffer, 0, VK_WHOLE_SIZE);
		const VkWriteDescriptorSet writeDescriptorSet = Create::WriteDescriptorSet(set, BUFFER_BINDING, bufferHandle.GetIndex(), 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &descriptorBufferInfo);
		vkUpdateDescriptorSets(*device, 1, &writeDescriptorSet, 0, nullptr);

		return bufferHandle;
	}
	TextureHandle ResourceManager::CreateTexture(const VkImageCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationCreateInfo)
	{
		if (samplers.size() == 0)
		{
			VkSamplerCreateInfo samplerInfo = Create::SamplerCreateInfo(
				VK_FILTER_LINEAR, VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, 1.0f, 1.0f
			);
			samplers.emplace_back(*device, samplerInfo);
		}

		TextureHandle textureHandle(textures.emplace(Vk::Image(*device, device->GetAllocator(), createInfo, allocationCreateInfo), samplers[0]));
		Texture* texturePtr = textures.get(textureHandle);

		CS_ASSERT(texturePtr != nullptr, "Failed to add texture to slotmap");

		const VkDescriptorImageInfo descriptorImageInfo = Create::DescriptorImageInfo(texturePtr->sampler, texturePtr->image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		const VkWriteDescriptorSet writeDescriptorSet = Create::WriteDescriptorSet(set, IMAGE_BINDING, textureHandle.GetIndex(), 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &descriptorImageInfo);
		vkUpdateDescriptorSets(*device, 1, &writeDescriptorSet, 0, nullptr);

		return textureHandle;
	}
	TextureHandle ResourceManager::UploadTexture(const cs_std::image& image, const TextureSpecification& textureSpec)
	{
		VkFormat format = GetImageFormat(textureSpec.colorspace, image.channels);

		/* ---------------------------------------------------------------- 0. - Data validation ---------------------------------------------------------------- */

		CS_ASSERT(format != VK_FORMAT_UNDEFINED, "Invalid image data. Image format is not supported");
		CS_ASSERT(image.width > 0 && image.height > 0, "Invalid image data. Image width and height must be greater than 0");

		/* ---------------------------------------------------------------- 1. - Sampler Creation ---------------------------------------------------------------- */
		// Create samplers up to the mip level of the image, these are reused for all textures
		const size_t imageSize = static_cast<size_t>(image.width) * static_cast<size_t>(image.height) * static_cast<size_t>(image.channels);
		const uint8_t mipLevels = 1 + (textureSpec.generateMipmaps ? static_cast<uint8_t>(std::log2(std::max(image.width, image.height))) : 0);
		VkSamplerCreateInfo samplerInfo = Create::SamplerCreateInfo(
			VK_FILTER_LINEAR, VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
			textureSpec.anisotropy, 1.0f
		);

		VkSampler sampler = nullptr;
		for (uint8_t i = static_cast<uint8_t>(samplers.size()); i < mipLevels; i++)
		{
			samplerInfo.maxLod = static_cast<float>(i);
			samplers.emplace_back(*device, samplerInfo);
		}
		sampler = samplers[mipLevels - 1];

		/* ----------------------------------------------------------------  2. - Staging ---------------------------------------------------------------- */
		Vk::Buffer staging(
			device->GetAllocator(),
			Create::BufferCreateInfo(0, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, nullptr),
			Create::AllocationCreateInfo(VMA_MEMORY_USAGE_CPU_ONLY)
		);
		staging.memcpy(image.data.data(), imageSize, 0);
		const VkExtent3D extent = Create::Extent3D(image.width, image.height, 1);
		Vk::Image texture(*device, device->GetAllocator(), Create::ImageCreateInfo(
			VK_IMAGE_TYPE_2D, format, extent,
			mipLevels, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_SHARING_MODE_EXCLUSIVE, 0, nullptr, VK_IMAGE_LAYOUT_UNDEFINED
		), Create::AllocationCreateInfo(VMA_MEMORY_USAGE_GPU_ONLY));

		/* ----------------------------------------------------------------  3. - MipMap Generation ---------------------------------------------------------------- */
		this->transferQueue.InstantSubmit([&](const Vk::TransferCommandQueue& cmd) {
			VkImageMemoryBarrier barrier = Vulkan::Create::ImageMemoryBarrier(
				0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, texture,
				Vulkan::Create::ImageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT, 0, mipLevels, 0, 1)
			);
			cmd.ResourceBarrier(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, barrier);
			const VkBufferImageCopy region = Vulkan::Create::BufferImageCopy(0, 0, 0, Vulkan::Create::ImageSubresourceLayers(VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1), Vulkan::Create::Offset3D(0, 0, 0), extent);
			cmd.CopyBufferToImage(staging, texture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, region);
			barrier = Vulkan::Create::ImageMemoryBarrier(
				0, 0, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_UNDEFINED,
				VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, texture,
				Vulkan::Create::ImageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1)
			);
			for (uint32_t i = 1, mipWidth = image.width, mipHeight = image.height; i < mipLevels; i++, mipWidth = std::max(mipWidth / 2, 1u), mipHeight = std::max(mipHeight / 2, 1u))
			{
				barrier.subresourceRange.baseMipLevel = i - 1;
				barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
				cmd.ResourceBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, barrier);
				const VkImageBlit blit = Vulkan::Create::ImageBlit(
					Vulkan::Create::ImageSubresourceLayers(VK_IMAGE_ASPECT_COLOR_BIT, i - 1, 0, 1), Vulkan::Create::Offset3D(0, 0, 0), Vulkan::Create::Offset3D(static_cast<int32_t>(mipWidth), static_cast<int32_t>(mipHeight), 1),
					Vulkan::Create::ImageSubresourceLayers(VK_IMAGE_ASPECT_COLOR_BIT, i, 0, 1), Vulkan::Create::Offset3D(0, 0, 0), Vulkan::Create::Offset3D(static_cast<int32_t>(std::max(mipWidth / 2, 1u)), static_cast<int32_t>(std::max(mipHeight / 2, 1u)), 1)
				);
				cmd.BlitImage(texture, texture, blit);
				barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
				cmd.ResourceBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, barrier);
			}
			barrier.subresourceRange.baseMipLevel = mipLevels - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			cmd.ResourceBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, barrier);
		});

		/* ----------------------------------------------------------------  4. - Handle Creation ---------------------------------------------------------------- */
		//return TextureHandle(textures.emplace(std::move(texture), sampler));

		TextureHandle textureHandle(textures.emplace(std::move(texture), sampler));
		Texture* texturePtr = textures.get(textureHandle);

		CS_ASSERT(texturePtr != nullptr, "Failed to add texture to slotmap");

		const VkDescriptorImageInfo descriptorImageInfo = Create::DescriptorImageInfo(texturePtr->sampler, texturePtr->image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		const VkWriteDescriptorSet writeDescriptorSet = Create::WriteDescriptorSet(set, IMAGE_BINDING, textureHandle.GetIndex(), 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &descriptorImageInfo);
		vkUpdateDescriptorSets(*device, 1, &writeDescriptorSet, 0, nullptr);

		return textureHandle;
	}
	template<> MeshHandle ResourceManager::UploadMesh(const cs_std::graphics::mesh<uint32_t>& mesh)
	{
		return TemplateUploadMesh<uint32_t>(mesh, this->device, this->transferQueue, this->meshes);
	}
	template<> MeshHandle ResourceManager::UploadMesh(const cs_std::graphics::mesh<uint16_t>& mesh)
	{
		return TemplateUploadMesh<uint16_t>(mesh, this->device, this->transferQueue, this->meshes);
	}
	Buffer& ResourceManager::GetBuffer(BufferHandle handle)
	{
		Buffer* buffer = buffers.get(handle);
		CS_ASSERT(buffer != nullptr, "Buffer handle is invalid");
		return *buffer;
	}
	Texture& ResourceManager::GetTexture(TextureHandle handle)
	{
		Texture* texture = textures.get(handle);
		CS_ASSERT(texture != nullptr, "Texture handle is invalid");
		return *texture;
	}
	Mesh& ResourceManager::GetMesh(MeshHandle handle)
	{
		Mesh* mesh = meshes.get(handle);
		CS_ASSERT(mesh != nullptr, "Mesh handle is invalid");
		return *mesh;
	}
	void ResourceManager::DestroyBuffer(BufferHandle handle) { buffers.erase(handle); }
	void ResourceManager::DestroyTexture(TextureHandle handle) { textures.erase(handle); }
	void ResourceManager::DestroyMesh(MeshHandle handle) { meshes.erase(handle); }
	size_t ResourceManager::GetBufferCount() const { return buffers.size(); }
	size_t ResourceManager::GetTextureCount() const { return textures.size(); }
	size_t ResourceManager::GetMeshCount() const { return meshes.size(); }
	VkDescriptorSetLayout ResourceManager::GetDescriptorSetLayout() const { return layout; }
	VkDescriptorSet ResourceManager::GetDescriptorSet() const { return set; }
}
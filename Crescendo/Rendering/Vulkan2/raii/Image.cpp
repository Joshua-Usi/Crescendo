#include "Image.hpp"
#include "Volk/volk.h"
#include "../Create.hpp"

CS_NAMESPACE_BEGIN::Vulkan::Vk
{
	Image::Image() : device(nullptr), allocator(nullptr), allocation(nullptr), image(nullptr), imageView(nullptr) {}
	Image::Image(VkDevice device, VmaAllocator allocator, const VkImageCreateInfo& createInfo, const VmaAllocationCreateInfo& allocationInfo) : device(device), allocator(allocator)
	{
		// Maps VmaMemoryUsage to VkMemoryPropertyFlags
		constexpr VkMemoryPropertyFlags FLAG_MAP[3] {
			0,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		};
		// Maps VkImageType to VkImageViewType
		constexpr VkImageViewType FORMAT_MAP[3] {
			VK_IMAGE_VIEW_TYPE_1D,
			VK_IMAGE_VIEW_TYPE_2D,
			VK_IMAGE_VIEW_TYPE_3D,
		};

		// Check if the image is a depth image, if so set the image flags
		VkImageCreateFlags imageFlags = 0;
		switch (createInfo.format)
		{
		case VK_FORMAT_D16_UNORM:
		case VK_FORMAT_D32_SFLOAT:
		case VK_FORMAT_D16_UNORM_S8_UINT:
		case VK_FORMAT_D24_UNORM_S8_UINT:
		case VK_FORMAT_D32_SFLOAT_S8_UINT:
			imageFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
			break;
		default:
			imageFlags = VK_IMAGE_ASPECT_COLOR_BIT;
		}

		// Generate image
		CS_ASSERT(vmaCreateImage(this->allocator, &createInfo, &allocationInfo, &this->image, &this->allocation, nullptr) == VK_SUCCESS, "Failed to create image!");

		// Generate image view create info
		const VkImageViewCreateInfo viewInfo = Create::ImageViewCreateInfo(
			image, FORMAT_MAP[createInfo.imageType], createInfo.format, {},
			Create::ImageSubresourceRange(imageFlags, 0, createInfo.mipLevels, 0, 1)
		);
		CS_ASSERT(vkCreateImageView(this->device, &viewInfo, nullptr, &this->imageView) == VK_SUCCESS, "Failed to create image view!");
	}
	Image::~Image()
	{
		if (this->device == nullptr) return;
		vkDestroyImageView(this->device, this->imageView, nullptr);
		vmaDestroyImage(this->allocator, this->image, this->allocation);
	}
	Image::Image(Image&& other) noexcept : device(other.device), allocator(other.allocator), allocation(other.allocation), image(other.image), imageView(other.imageView)
	{
		other.device = nullptr;
		other.allocator = nullptr;
		other.allocation = nullptr;
		other.image = nullptr;
		other.imageView = nullptr;
	}
	Image& Image::operator=(Image&& other) noexcept
	{
		if (this == &other) return *this;
		this->device = other.device; other.device = nullptr;
		this->allocator = other.allocator; other.allocator = nullptr;
		this->allocation = other.allocation; other.allocation = nullptr;
		this->image = other.image; other.image = nullptr;
		this->imageView = other.imageView; other.imageView = nullptr;
		return *this;
	}
	VkImage Image::GetImage() { return image; }
	VkImageView Image::GetImageView() { return imageView; }
	Image::operator VkImage() const { return image; }
	Image::operator VkImageView() const { return imageView; }
}
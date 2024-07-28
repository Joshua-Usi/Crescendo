#include "ImageView.hpp"
#include "Volk/volk.h"

CS_NAMESPACE_BEGIN::Vulkan::Vk
{
	ImageView::ImageView() : device(nullptr), imageView(nullptr) {}
	ImageView::ImageView(VkDevice device, const VkImageViewCreateInfo& createInfo) : device(device)
	{
		CS_ASSERT(vkCreateImageView(this->device, &createInfo, nullptr, &this->imageView) == VK_SUCCESS, "Failed to create image view!");
	}
	ImageView::ImageView(VkDevice device, VkImageView imageView) : device(device), imageView(imageView) {}
	ImageView::~ImageView()
	{
		if (this->device != nullptr) vkDestroyImageView(this->device, this->imageView, nullptr);
	}
	ImageView::ImageView(ImageView&& other) noexcept : device(other.device), imageView(other.imageView)
	{
		other.device = nullptr;
		other.imageView = nullptr;
	}
	ImageView& ImageView::operator=(ImageView&& other) noexcept
	{
		if (this == &other) return *this;
		this->device = other.device; other.device = nullptr;
		this->imageView = other.imageView; other.imageView = nullptr;
		return *this;
	}
	ImageView::operator VkImageView() const { return imageView; }
	VkImageView ImageView::GetImageView() const { return imageView; }
}
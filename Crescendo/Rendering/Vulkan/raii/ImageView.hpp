#pragma once

#include "common.hpp"
#include "vulkan/vulkan.h"

CS_NAMESPACE_BEGIN::Vulkan::Vk
{
	class ImageView
	{
	private:
		VkDevice device;
		VkImageView imageView;
	public:
		ImageView();
		// Creates an image view from createInfo
		ImageView(VkDevice device, const VkImageViewCreateInfo& createInfo);
		// Takes ownership of imageView
		explicit ImageView(VkDevice device, VkImageView imageView);
		~ImageView();
		ImageView(const ImageView&) = delete;
		ImageView& operator=(const ImageView&) = delete;
		ImageView(ImageView&& other) noexcept;
		ImageView& operator=(ImageView&& other) noexcept;
		operator VkImageView() const;
		VkImageView GetImageView() const;
	};
}
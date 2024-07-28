#include "Swapchain.hpp"
#include "VkBootstrap/VkBootstrap.h"
#include "../Create.hpp"

CS_NAMESPACE_BEGIN::Vulkan::Vk
{
	Swapchain::Swapchain() : device(nullptr), swapchain(nullptr) {}
	Swapchain::Swapchain(const PhysicalDevice& physicalDevice, const Device& device, const Surface& surface, const SwapchainSpecification& spec) : device(device)
	{
		vkb::Result<vkb::Swapchain> vkbSwapchain = vkb::SwapchainBuilder(physicalDevice, device, surface)
			.use_default_format_selection()
			.set_desired_present_mode(spec.presentMode)
			.set_desired_extent(spec.windowExtent.width, spec.windowExtent.height)
			.build();
		CS_ASSERT(vkbSwapchain, "Failed to create swapchain! " + vkbSwapchain.error().message());

		this->swapchain = vkbSwapchain.value().swapchain;
		this->imageFormat = vkbSwapchain.value().image_format;
		this->extent = vkbSwapchain.value().extent;
		this->renderPass = this->CreateRenderpass(spec.samples);

		std::vector<VkImage> SCimages = vkbSwapchain.value().get_images().value();
		std::vector<VkImageView> SCimageViews = vkbSwapchain.value().get_image_views().value();
		const uint32_t imageCount = static_cast<uint32_t>(SCimages.size());
		this->images.resize(imageCount);
		for (size_t i = 0; i < imageCount; i++)
		{
			this->images[i] = Image(nullptr, SCimages[i], SCimageViews[i]);
			VkFramebufferCreateInfo framebufferInfo = Create::FramebufferCreateInfo(this->renderPass, &SCimageViews[i], this->extent, 1);
			CS_ASSERT(vkCreateFramebuffer(this->device, &framebufferInfo, nullptr, &this->images[i].framebuffer) == VK_SUCCESS, "Failed to create framebuffer!");
		}
	}
	Swapchain::Swapchain(const Device& device, VkSwapchainKHR swapchain) : device(device), swapchain(swapchain) {}
	Swapchain::~Swapchain()
	{
		if (this->device == nullptr) return;
		for (auto& image : this->images)
		{
			vkDestroyFramebuffer(this->device, image.framebuffer, nullptr);
			vkDestroyImageView(this->device, image.imageView, nullptr);
			// Swapchain images are not owned by us, it will be destroyed by the swapchain
		}
		vkDestroySwapchainKHR(this->device, this->swapchain, nullptr);
	}
	Swapchain::Swapchain(Swapchain&& other) noexcept : device(other.device), swapchain(other.swapchain), imageFormat(other.imageFormat), extent(other.extent), renderPass(std::move(other.renderPass)), images(std::move(other.images))
	{
		other.device = nullptr;
		other.swapchain = nullptr;
		other.images.clear();
	}
	Swapchain& Swapchain::operator=(Swapchain&& other) noexcept
	{
		if (this == &other) return *this;
		this->device = other.device; other.device = nullptr;
		this->swapchain = other.swapchain; other.swapchain = nullptr;
		this->imageFormat = other.imageFormat; other.imageFormat = VK_FORMAT_UNDEFINED;
		this->extent = other.extent; other.extent = { 0, 0 };
		this->renderPass = std::move(other.renderPass);
		this->images = std::move(other.images); other.images.clear();
		return *this;
	}

	RenderPass Swapchain::CreateRenderpass(VkSampleCountFlagBits samples)
	{
		VkAttachmentDescription attachment = Create::AttachmentDescription(
			imageFormat, samples,
			VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
		);
		VkAttachmentReference attachmentRef = Create::AttachmentReference(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		VkSubpassDescription subpass = Create::SubpassDescription(
			VK_PIPELINE_BIND_POINT_GRAPHICS, nullptr, &attachmentRef, nullptr, nullptr, nullptr
		);
		VkSubpassDependency dependency = Create::SubpassDependency(
			VK_SUBPASS_EXTERNAL, 0,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 0
		);

		const VkRenderPassCreateInfo renderPassInfo = Create::RenderPassCreateInfo(&attachment, &subpass, &dependency);
		return RenderPass(device, renderPassInfo);
	}
	VkResult Swapchain::AcquireNextImage(VkSemaphore imageAvailableSemaphore, uint64_t timeout)
	{
		return vkAcquireNextImageKHR(this->device, this->swapchain, timeout, imageAvailableSemaphore, VK_NULL_HANDLE, &this->acquiredImageIndex);
	}
	VkResult Swapchain::Present(VkQueue queue, VkSemaphore renderFinishSemaphore)
	{
		const VkPresentInfoKHR presentInfo = Create::PresentInfoKHR(&renderFinishSemaphore, &this->swapchain, &this->acquiredImageIndex);
		return vkQueuePresentKHR(queue, &presentInfo);
	}
	Swapchain::operator VkSwapchainKHR() const { return swapchain; }
	VkSwapchainKHR Swapchain::GetSwapchain() const { return swapchain; }
	size_t Swapchain::ImageCount() const { return images.size(); }
	VkImage Swapchain::GetImage(uint32_t index) const { return images[index].image; }
	VkImageView Swapchain::GetImageView(uint32_t index) const { return images[index].imageView; }
	Swapchain::Image& Swapchain::GetFramebuffer(uint32_t index) { return images[index]; }
	VkFormat Swapchain::GetImageFormat() const { return imageFormat; }
	const VkExtent2D& Swapchain::GetExtent() const { return extent; }
	VkExtent3D Swapchain::GetExtent3D() const { return { extent.width, extent.height, 1 }; }
	VkViewport Swapchain::GetViewport(bool flipY) const { return { 0.0f, flipY ? static_cast<float>(extent.height) : 0.0f, static_cast<float>(extent.width), flipY ? -static_cast<float>(extent.height) : static_cast<float>(extent.height), 0.0f, 1.0f }; }
	VkRect2D Swapchain::GetScissor() const { return { { 0, 0 }, extent }; }
	uint32_t Swapchain::GetAcquiredImageIndex() const { return acquiredImageIndex; }
	VkRenderPass Swapchain::GetRenderPass() const { return renderPass; }
}
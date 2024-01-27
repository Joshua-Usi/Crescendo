#include "Swapchain.hpp"
#include "VkBootstrap/VkBootstrap.h"
#include "Types/Create.hpp"

#include "Surface.hpp"
#include "Device.hpp"

CS_NAMESPACE_BEGIN::Vulkan
{
	Swapchain::Swapchain(const Surface& surface, const Device& device, VkPresentModeKHR presentMode, VkExtent2D windowExtent)
	: renderPass(nullptr)
	{
		vkb::Swapchain vkbSwapchain = vkb::SwapchainBuilder(surface.GetVkPhysicalDevice(), device, surface)
			.use_default_format_selection()
			.set_desired_present_mode(presentMode)
			.set_desired_extent(windowExtent.width, windowExtent.height)
			.build().value();

		this->device = device;
		this->swapchain = vkbSwapchain.swapchain;
		this->imageFormat = vkbSwapchain.image_format;
		this->extent = vkbSwapchain.extent;
		this->renderPass = this->CreateRenderpass(VK_SAMPLE_COUNT_1_BIT);

		// Merge into images
		std::vector<VkImage> SCimages = vkbSwapchain.get_images().value();
		std::vector<VkImageView> SCimageViews = vkbSwapchain.get_image_views().value();
		const uint32_t imageCount = static_cast<uint32_t>(SCimages.size());
		this->framebuffers.resize(imageCount);
		for (size_t i = 0; i < imageCount; i++)
		{
			VkFramebuffer fb = nullptr;
			VkFramebufferCreateInfo framebufferInfo = Create::FramebufferCreateInfo(this->renderPass, &SCimageViews[i], this->extent, 1);
			CS_ASSERT(vkCreateFramebuffer(this->device, &framebufferInfo, nullptr, &fb) == VK_SUCCESS, "Failed to create framebuffer!");
			this->framebuffers[i] = Framebuffer(fb, SCimages[i], SCimageViews[i]);
		}
	}
	Swapchain::~Swapchain()
	{
		if (this->device == nullptr) return;
		for (auto& framebuffer : this->framebuffers)
		{
			vkDestroyFramebuffer(this->device, framebuffer.framebuffer, nullptr);
			vkDestroyImageView(this->device, framebuffer.imageView, nullptr);
			// Swapchain images are not owned by us, it will be destroyed by the swapchain
		}
		vkDestroyRenderPass(this->device, this->renderPass, nullptr);
		vkDestroySwapchainKHR(this->device, this->swapchain, nullptr);
	}
	Swapchain::Swapchain(Swapchain&& other) noexcept
		:	device(other.device), swapchain(other.swapchain), imageFormat(other.imageFormat), renderPass(other.renderPass),
			extent(other.extent), framebuffers(std::move(other.framebuffers)), needsRecreation(other.needsRecreation)
	{
		other.device = nullptr;
		other.framebuffers.clear();
	}
	Swapchain& Swapchain::operator=(Swapchain&& other) noexcept
	{
		if (this == &other) return *this;
		this->device = other.device;
		this->swapchain = other.swapchain;
		this->imageFormat = other.imageFormat;
		this->renderPass = other.renderPass;
		this->extent = other.extent;
		this->framebuffers = std::move(other.framebuffers);
		this->needsRecreation = other.needsRecreation;
		other.device = nullptr;
		other.framebuffers.clear();
		return *this;
	}
	uint32_t Swapchain::AcquireNextImage(VkSemaphore signalSemaphore, uint64_t timeout)
	{
		uint32_t index;
		VkResult imageAcquireResult = vkAcquireNextImageKHR(this->device, this->swapchain, timeout, signalSemaphore, VK_NULL_HANDLE, &index);
		this->needsRecreation = imageAcquireResult == VK_ERROR_OUT_OF_DATE_KHR || imageAcquireResult == VK_SUBOPTIMAL_KHR;
		CS_ASSERT(!this->needsRecreation && imageAcquireResult == VK_SUCCESS, "Failed to acquire swap chain image!");
		return index;
	}
	VkRenderPass Swapchain::CreateRenderpass(VkSampleCountFlagBits samples)
	{
		if (this->renderPass != nullptr) vkDestroyRenderPass(this->device, this->renderPass, nullptr);
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

		VkRenderPass renderPass;
		CS_ASSERT(vkCreateRenderPass(this->device, &renderPassInfo, nullptr, &renderPass) == VK_SUCCESS, "Failed to create render pass");
		return renderPass;
	}

}
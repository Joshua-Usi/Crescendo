#include "DepthPrePass.hpp"
#include "PostProcessingPass.hpp"
#include "OffscreenPass.hpp"

#include "Engine/Application/Application.hpp"
#include "Rendering/Vulkan2/Create.hpp"
#include "Engine/Scene/Scene.hpp"

#include "Engine/CVar/Cvar.hpp"

#include "cs_std/file.hpp"

CS_NAMESPACE_BEGIN
{
	DepthPrePass::DepthPrePass(Vulkan::Surface& surface, const std::string& shaderName, VkFormat depthFormat, VkSampleCountFlagBits samples) : surface(surface)
	{
		Vulkan::Device& device = surface.GetDevice();

		VkAttachmentDescription depthAttachment = Vulkan::Create::AttachmentDescription(
			depthFormat, samples,
			VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		);
		VkAttachmentReference depthAttachmentRef = Vulkan::Create::AttachmentReference(0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		VkSubpassDescription depthSubpass = Vulkan::Create::SubpassDescription(VK_PIPELINE_BIND_POINT_GRAPHICS, nullptr, nullptr, nullptr, &depthAttachmentRef, nullptr);
		VkSubpassDependency depthDependency = Vulkan::Create::SubpassDependency(
			VK_SUBPASS_EXTERNAL, 0, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, VK_ACCESS_SHADER_READ_BIT,
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_DEPENDENCY_BY_REGION_BIT
		);
		VkSubpassDependency depthDependency2 = Vulkan::Create::SubpassDependency(
			0, VK_SUBPASS_EXTERNAL, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			VK_ACCESS_SHADER_READ_BIT, VK_DEPENDENCY_BY_REGION_BIT
		);

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
		this->renderPass = Vulkan::Vk::RenderPass(device, Vulkan::Create::RenderPassCreateInfo(&depthAttachment, &depthSubpass, depthDependencies));

		const Vulkan::Vk::PipelineVariants variant = Vulkan::Vk::PipelineVariants::GetDepthPrepassVariant();
		const VkDescriptorSetLayout layout = surface.GetDevice().GetBindlessDescriptorManager().GetLayout();
		this->depthPipeline = Vulkan::Vk::Pipeline(surface.GetDevice(), {
			cs_std::binary_file(shaderName + ".vert.spv").open().read_if_exists(),
			{},
			variant, layout, renderPass
		});
	}
	void DepthPrePass::Execute(Vulkan::Vk::GraphicsCommandQueue& cmd)
	{
		Vulkan::Vk::Swapchain& swapchain = surface.GetSwapchain();

		cmd.DynamicStateSetViewport(swapchain.GetViewport());
		cmd.DynamicStateSetScissor(swapchain.GetScissor());

		//cmd.BeginRenderPass();
		//cmd.EndRenderPass();
	}
	PostProcessPass::PostProcessPass(Vulkan::Surface& surface, const std::string& shaderName) : surface(surface)
	{
		const VkRenderPass renderPass = surface.GetSwapchain().GetRenderPass();
		const Vulkan::Vk::PipelineVariants variant = Vulkan::Vk::PipelineVariants::GetPostProcessingVariant();
		const VkDescriptorSetLayout layout = surface.GetDevice().GetBindlessDescriptorManager().GetLayout();
		pipeline = Vulkan::Vk::Pipeline(surface.GetDevice(), {
			cs_std::binary_file(shaderName + ".vert.spv").open().read_if_exists(),
			cs_std::binary_file(shaderName + ".frag.spv").open().read_if_exists(),
			variant, layout, renderPass
		});
	}
	void PostProcessPass::Execute(Vulkan::Vk::GraphicsCommandQueue& cmd)
	{
		Vulkan::Vk::Swapchain& swapchain = surface.GetSwapchain();
		const uint32_t imageIndex = swapchain.GetAcquiredImageIndex();
		const VkDescriptorSet set = surface.GetDevice().GetBindlessDescriptorManager().GetSet();

		cmd.DynamicStateSetViewport(swapchain.GetViewport());
		cmd.DynamicStateSetScissor(swapchain.GetScissor());
		cmd.BeginRenderPass(
			swapchain.GetRenderPass(), swapchain.GetFramebuffer(imageIndex).framebuffer, swapchain.GetScissor(),
			{ Vulkan::Create::ClearValue(1.0f, 0.0f, 0.0f, 1.0f), Vulkan::Create::DefaultDepthClear() }
		);
		cmd.BindPipeline(pipeline[0]);
		cmd.BindDescriptorSet(pipeline, set, 0, 0, false);
		uint32_t imgIdx = 1;
		cmd.PushConstants(pipeline, imgIdx, VK_SHADER_STAGE_FRAGMENT_BIT);
		cmd.Draw(6);
		cmd.EndRenderPass();
	}
	OffscreenPass::OffscreenPass()
	{

	}
	void OffscreenPass::Execute(Vulkan::Vk::GraphicsCommandQueue& cmd)
	{

	}
}
#pragma once
#include "common.hpp"

#include "Vulkan/raii.hpp"
#include "ResourceHandles.hpp"
#include "Font.hpp"
#include "Vulkan/Instance.hpp"
#include "Vulkan/FrameManager.hpp"
#include "RenderResourceManager.hpp"
#include "Scene/Scene.hpp"

CS_NAMESPACE_BEGIN
{
	class Renderer
	{
	private:
		Vulkan::Instance instance;
	public:
		RenderResourceManager resourceManager;
	private:
		Vulkan::FrameManager frameManager;

		Vulkan::PipelineHandle postProcessingPipelineHandle;

		Vulkan::RenderPassHandle depthRenderPassHandle;
		Vulkan::FramebufferHandle depthFramebufferHandle;
		Vulkan::TextureHandle depthImageHandle;
		Vulkan::PipelineHandle depthPipelineHandle;

		Vulkan::RenderPassHandle mainRenderPassHandle;
		Vulkan::PipelineHandle mainPipelineHandle;
		Vulkan::TextureHandle mainImageHandle;
		Vulkan::FramebufferHandle mainFramebufferHandle;

		std::vector<Vulkan::SSBOBufferHandle> transformsHandle;

		Vulkan::PipelineHandle particlePipelineHandle;
		std::vector<Vulkan::SSBOBufferHandle> particleBufferHandle;

		std::vector<Vulkan::SSBOBufferHandle> directionalLightsHandle;
		std::vector<Vulkan::SSBOBufferHandle> pointLightsHandle;
		std::vector<Vulkan::SSBOBufferHandle> spotLightsHandle;

		Vulkan::PipelineHandle skyboxPipelineHandle;
		Vulkan::Mesh skyboxMesh;

		Vulkan::PipelineHandle textPipelineHandle;
		std::unordered_map<std::string, Font> fonts;
		std::vector<Vulkan::SSBOBufferHandle> textAdvanceDataHandle;
		// Each individual character
		std::vector<Vulkan::SSBOBufferHandle> textCharacterDataHandle;

		Vulkan::PipelineHandle bloomDownsamplePipelineHandle, bloomUpsamplePipelineHandle;
		Vulkan::RenderPassHandle bloomRenderPassHandle;
		// first framebuffer is the composite image, the rest are the downsampled images
		std::vector<Vulkan::FramebufferHandle> bloomFramebufferHandles;
		std::vector<Vulkan::TextureHandle> bloomImageHandles;
	public:
		Renderer() = default;
		~Renderer() = default;
		void Init();
		void RenderScene(Scene& scene);
		void WaitIdle();
	};
}
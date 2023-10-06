#include "Renderer.hpp"

#include "./RendererImpl/RendererImpl.hpp"

namespace Crescendo
{
	Renderer::Renderer() : impl(std::make_unique<RendererImpl>()) {}
	Renderer::~Renderer() = default;
	Renderer::Renderer(Renderer&& other) noexcept : impl(std::move(other.impl)) {}
	Renderer& Renderer::operator=(Renderer&& other) noexcept
	{
		this->impl = std::move(other.impl); return *this;
	}

	void Renderer::Init(const BuilderInfo& info)
	{
		// Validate the info
		CS_ASSERT(info.window != nullptr, "Window pointer points to nullptr!");
		CS_ASSERT(info.windowExtent.width > 0, "Window width must be greater than 0!");
		CS_ASSERT(info.windowExtent.height > 0, "Window height must be greater than 0!");
		CS_ASSERT((info.msaaSamples & (info.msaaSamples - 1)) == 0 || info.msaaSamples == std::numeric_limits<uint32_t>::max(), "MSAA samples must be a power of 2!");
		CS_ASSERT(info.anistropicFiltering >= 1.0f, "Anisotropic filtering must be greater than 0!");
		CS_ASSERT(info.framesInFlight > 0, "Frames in flight must be greater than 0!");
		CS_ASSERT(info.vertexBufferBlockSize > 0, "Vertex buffer block size must be greater than 0!");
		CS_ASSERT(info.descriptorBufferBlockSize > 0, "Descriptor buffer block size must be greater than 0!");

		// Fixed initialisation, these are never changable at runtime
		CS_TIME(this->impl->InitialiseInstance(info), "Instance Initialisation");
		CS_TIME(this->impl->InitialiseTransferQueues(info), "Transfer queue Initialisation");
		CS_TIME(this->impl->InitialiseDescriptors(info), "Descriptors Initialisation");
		CS_TIME(this->impl->InitialisePipelines(info), "Pipelines Initialisation");
		CS_TIME(this->impl->InitialiseBuffers(info), "Buffer initialisation");

		// Variable initialisation, these can be changed at runtime and reinitialised
		CS_TIME(this->impl->InitialiseFlightFrames(info), "Command queue Initialisation");
		CS_TIME(this->impl->InitialiseSwapchain(info), "Swapchain Initialisation");
		CS_TIME(this->impl->InitialiseFramebuffers(info), "Framebuffers Initialisation");
	}

	void Renderer::CmdBindTexture(uint32_t set, uint32_t texture)
	{
		this->impl->BindTexture(set, texture);
	}
	void Renderer::CmdUpdatePushConstant(ShaderStage stage, const void* data, uint32_t size)
	{
		this->impl->UpdatePushConstant(stage, data, size);
	}
	void Renderer::UpdateDescriptorSetData(uint32_t descriptorSetIndex, uint32_t binding, const void* data, uint32_t size)
	{
		this->impl->UpdateDescriptorSet(descriptorSetIndex, binding, data, size);
	}

	void Renderer::CmdBeginFrame()
	{
		this->impl->BeginFrame();
	}
	void Renderer::CmdEndFrame()
	{
		this->impl->EndFrame();
	}
	void Renderer::CmdBeginRenderPass(uint32_t renderPassIndex, float r, float g, float b, float a)
	{
		this->impl->BeginRenderPass(renderPassIndex, { r, g, b, a });
	}
	void Renderer::CmdEndRenderPass()
	{
		this->impl->EndRenderPass();
	}

	void Renderer::CmdBindPipeline(uint32_t pipelineIndex)
	{
		this->impl->BindPipeline(pipelineIndex);
	}
	void Renderer::CmdDraw(uint32_t mesh)
	{
		this->impl->Draw(mesh);
	}
	void Renderer::CmdPresentFrame()
	{
		this->impl->PresentFrame();
	}

	void Renderer::Resize()
	{
		this->impl->Resize();
	}
	
	void Renderer::UploadMesh(const std::vector<ShaderAttribute>& attributes, const std::vector<uint32_t>& indices)
	{
		this->impl->UploadMesh(attributes, indices);
	}
	void Renderer::UploadPipeline(const std::vector<uint8_t>& vertexShader, const std::vector<uint8_t>& fragmentShader, const std::vector<PipelineVariant>& variants)
	{
		this->impl->UploadPipeline(vertexShader, fragmentShader, variants);
	}
	void Renderer::UploadTexture(const void* textureData, uint32_t width, uint32_t height, uint32_t channels, bool generateMipmaps)
	{
		this->impl->UploadTexture(textureData, width, height, channels, generateMipmaps);
	}
	
	Renderer Renderer::Create(const Renderer::BuilderInfo& info)
	{
		Renderer renderer {};
		renderer.Init(info);
		return renderer;
	}
	void Renderer::Destroy(Renderer& renderer)
	{
		if (renderer.impl == nullptr) return;
		vkDeviceWaitIdle(renderer.impl->device);
		renderer.impl->mainDeletionQueue.flush();
	}
}
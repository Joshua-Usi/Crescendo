#pragma once

#include "Core/common.hpp"

#include <vector>

namespace Crescendo
{
	class Renderer
	{
	private:
		// PIMPL idiom to hide implementation details
		// I want to keep the header namespace clean so nothing vulkan is exposed
		class RendererImpl;
		unique<RendererImpl> impl;
	public:
		enum class ShaderStage { Vertex, Fragment };
		/// <summary>
		/// Outlines instructions for how the renderer should be built
		/// </summary>
		struct BuilderInfo
		{
			enum class DeviceType : uint32_t { Discrete = 0, Integrated = 1 };
			enum class PresentMode : uint32_t
			{
				Mailbox = 0, // The displayed image will always be the most recent image
				FIFO = 1 // Images are placed in a queue and the oldest image is displayed. Guaranteed to be supported
			};
			struct WindowExtent { uint32_t width, height; };
			// Whether or not the application should use validation layers
			bool useValidationLayers;
			// Whether or not the applicaton should prefer to use a discrete or integrated GPU
			DeviceType preferredDeviceType;
			// Application specifics that can allows driver optimisations
			std::string appName, engineName;
			// The window to render to, surface is created from this
			void* window;
			// Window framebuffer size
			WindowExtent windowExtent;
			PresentMode preferredPresentMode;
			/*
			 *	Number of frames in flight. For each frame in flight, the following resources are allocated:
			 *	Command Pool, Command Buffer, Descriptor data buffer, Descriptor Set
			 */
			uint32_t framesInFlight;
			/*
			 *	Specifies the minimum vertex attribute buffer size when creating a new buffer, If a mesh is larger than this
			 *	It will create a buffer that is a multiple of this size
			 *  Larger buffers increase initial memory usage but decrease binds
			 *	Size is in bytes
			 */
			uint64_t vertexBufferBlockSize;
			/*
			 *	Specfies the maximum size of the descriptor buffer for 1 frame that can be stored without resizing
			 *	Size is in bytes
			 */
			uint64_t descriptorBufferBlockSize;

			BuilderInfo() = default;
		};
		// Generates variations of a pipeline based on the given information
		struct PipelineVariant
		{
			enum class FillMode : uint8_t { Solid = 0, Wireframe = 1, Point = 2 };
			FillMode fillMode;
			bool depthTestEnable;
			bool depthWriteEnable;
			inline PipelineVariant(
				FillMode fillMode = FillMode::Solid,
				bool depthTestEnable = true,
				bool depthWriteEnable = true
			) : fillMode(fillMode),
				depthTestEnable(depthTestEnable),
				depthWriteEnable(depthWriteEnable) {}
		};
	private:
	public:
		Renderer();
		~Renderer();

		// Delete copy and copy assignment constructors
		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;

		// Define move constructors
		Renderer(Renderer&& other) noexcept;
		Renderer& operator=(Renderer&& other) noexcept;

		void Init(const BuilderInfo& info);

		void CmdBeginFrame(float r, float g, float b, float a);
		void CmdEndFrame();
		void CmdBindPipeline(uint32_t pipelineIndex);
		void CmdUpdatePushConstant(ShaderStage stage, const void* data, uint32_t size);
		template<typename T>
		void CmdUpdatePushConstant(ShaderStage stage, const T& data) { this->CmdUpdatePushConstant(stage, &data, sizeof(data)); }
		void CmdDraw(uint32_t mesh);
		void CmdPresentFrame();

		void UpdateDescriptorSetData(uint32_t descriptorSetIndex, uint32_t binding, const void* data, uint32_t size);
		template<typename T>
		void UpdateDescriptorSetData(uint32_t descriptorSetIndex, uint32_t binding, const T& data) { this->UpdateDescriptorSetData(descriptorSetIndex, binding, &data, sizeof(data)); }

		void Resize();

		void UploadMesh(const std::vector<float>& vertices, const std::vector<float>& normals, const std::vector<float>& textureUVs, const std::vector<uint32_t>& indices);
		void UploadPipeline(const std::vector<uint8_t>& vertexShader, const std::vector<uint8_t>& fragmentShader, const std::vector<PipelineVariant>& variations = { {} });
		void UploadTexture(const std::vector<uint8_t>& data, uint32_t width, uint32_t height, uint32_t channels);

		static Renderer Create(const BuilderInfo& info);
		static void Destroy(Renderer& renderer);
	};
}
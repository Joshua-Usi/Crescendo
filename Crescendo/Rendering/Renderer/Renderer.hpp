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
		const static uint32_t SHADOW_MAP_ID = UINT32_MAX;
		enum class ShaderStage { Vertex, Fragment };
		enum class ShaderAttributeFlag : uint32_t
		{
			Position = 0,
			Normal,
			Tangent,
			TexCoord_0,
			TexCoord_1,
			Color_0,
			Joints_0,
			Weights_0,
			SHADER_ATTRIBUTE_FLAG_COUNT,
		};
		struct ShaderAttribute
		{
			std::vector<float> data;
			ShaderAttributeFlag attribute;
		};
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
			/*
			 *	Configured whether or not the engine should use the vulkan validation layers
			 *	Validation layers are used to check for errors in the application
			 *	Highly recommended during debugging or debug builds but it can slow down the application
			 *	Turn off for release
			 * 
			 *	Set during initialisation
			 */
			bool useValidationLayers;
			/*
			 *	Specifies if the engine should "prefer" a discrete or integrated GPU
			 *	Preferences aren't guaranteed but it will try to use the preferred device type
			 *
			 *	Set during initialisation
			 */
			DeviceType preferredDeviceType;
			/*
			 *	Companies can sometimes release game-specific drivers dedicated for a specific game or engine
			 *	
			 *	Set during initialisation
			 */
			std::string appName, engineName;
			/*
			 *	Specifies the window to render to, the engine builds the surface from this and uses it to detect frame buffer size changes
			 * 
			 *	Set during initialisation
			 */
			void* window;
			/*
			 *	The default and initalise size of the window. Will automatically change if the window is resized
			 *	Must be greater than 0
			 * 
			 *	Can be changed at runtime
			 */
			WindowExtent windowExtent;
			/*
			 *	Specifies a preferred present mode for the engine
			 *	Preferences aren't guaranteed but it will try to use the preferred present mode
			 *	Only FIFO is gauranteed support and that will be the fallback if the preferred mode is not supported
			 * 
			 *  Can be changed at runtime
			 */
			PresentMode preferredPresentMode;
			/*
			 *	Configures the number of frames in flight the gpu supports. Highly recommended to have between 2 and 3 frames in flight
			 *	More frames in flight means the CPU and GPU can work independently, So no cycles are wasted, However more frames in flight leads to more resource allocations
			 *	Which can increase memory usage.
			 * 
			 *	Can be changed at runtime
			 */
			uint32_t framesInFlight;
			/*
			 *	Specifies the base block size of the vertex attribute data. When vertex data is uploaded to the engine
			 *	The engine will automatically find a free space for it, if not, then it will allocate a new block equal
			 *	To the size of this variable. If the block required is larger than this variable, then the block will be
			 *	Allocated to be a multiple of this variable
			 *	Larger blocks increase initial memory usage but decrease binds
			 *	Size is in bytes, highly recommended to be a multiple of 2, Must be greater than 0
			 * 
			 *  Set during initialisation
			 */
			uint64_t vertexBufferBlockSize;
			/*
			 *	Specifies the base block size of the descriptor set data. When descriptor set data is uploaded to the engine
			 *	The engine will automatically find a free space for it, if not, then it will allocate a new block equal
			 *	To the size of this variable. If the block required is larger than this variable, then the block will be
			 *	Allocated to be a multiple of this variable
			 *	Size is in bytes, highly recommended to be a multiple of 2, Must be greater than 0
			 * 
			 *	Set during initialisation
			 */
			uint64_t descriptorBufferBlockSize;
			/*
			 *	Specifies the number of samples to use for multisampling
			 *	Multisampling is a technique used to reduce aliasing, it does come with a performance cost
			 *	Set to 1 for no multisampling, if set higher than the highest supported value, it will be clamped to the highest supported value
			 *	Must be a multiple of 2. Must not be equal to 0
			 * 
			 *	Can be changed at runtime
			 */
			uint32_t msaaSamples;
/*
			 *	Specifies the number of samples to use for anistropic filtering
			 *	Anistropic filtering is a technique used to reduce aliasing, it does come with a performance cost
			 *	Set to 1.0f for no anistropic filtering, if set higher than the highest supported value, it will be clamped to the highest supported value
			 *	Mostly set to a power of 2, Must not be less than 1.0f
			 * 
			 *	Set during initialisation (for now)
			 */
			float anistropicFiltering;
			/*
			 *	Number of descriptor sets per pool the auto-allocator allows
			 *	If a descriptor set is requested, it finds the next open compatible pool,
			 *	If there is no open pool, it creates a new one
			 * 
			 *	Set during initialisation
			 */
			uint32_t desriptorSetsPerPool;
			/*
			 *	Resolution of the shadowmap
			 *	Higher resolutions increase quality but decrease performance
			 *	Set to 0 for no shadowmap, Highly recommended to be a multiple of 2
			 * 
			 *	Can be changed at runtime
			 */
			uint32_t shadowMapResolution;

			BuilderInfo() = default;
		};
		// Generates variations of a pipeline based on the given information
		struct PipelineVariant
		{
			enum class FillMode : uint8_t { Solid = 0, Wireframe = 1, Point = 2 };
			enum class DepthFunc : uint8_t { Never = 0, Less = 1, Equal = 2, LessEqual = 3, Greater = 4, NotEqual = 5, GreaterEqual = 6, Always = 7 };
			enum class CullMode : uint8_t { None = 0, Front = 1, Back = 2 };
			enum class RenderPass : uint8_t { Default = 0, Shadow = 1 };
			FillMode fillMode;
			bool depthTestEnable;
			bool depthWriteEnable;
			DepthFunc depthFunc;
			CullMode cullMode;
			RenderPass renderPass;
			inline PipelineVariant(
				FillMode fillMode = FillMode::Solid,
				bool depthTestEnable = true,
				bool depthWriteEnable = true,
				DepthFunc depthFunc = DepthFunc::Less,
				CullMode cullMode = CullMode::Back,
				RenderPass renderPass = RenderPass::Default
			) : fillMode(fillMode),
				depthTestEnable(depthTestEnable),
				depthWriteEnable(depthWriteEnable),
				depthFunc(depthFunc),
				cullMode(cullMode),
				renderPass(renderPass)
				{}
		};
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
		void CmdBindTexture(uint32_t set, uint32_t texture);
		void CmdUpdatePushConstant(ShaderStage stage, const void* data, uint32_t size);
		template<typename T>
		void CmdUpdatePushConstant(ShaderStage stage, const T& data)
		{
			this->CmdUpdatePushConstant(stage, &data, sizeof(data));
		}
		void CmdDraw(uint32_t mesh);
		void CmdPresentFrame();

		void UpdateDescriptorSetData(uint32_t descriptorSetIndex, uint32_t binding, const void* data, uint32_t size);
		template<typename T>
		void UpdateDescriptorSetData(uint32_t descriptorSetIndex, uint32_t binding, const T& data)
		{
			this->UpdateDescriptorSetData(descriptorSetIndex, binding, &data, sizeof(data));
		}

		void Resize();

		void UploadMesh(const std::vector<ShaderAttribute>& attributes, const std::vector<uint32_t>& indices);
		void UploadPipeline(const std::vector<uint8_t>& vertexShader, const std::vector<uint8_t>& fragmentShader, const PipelineVariant& variant = {});
		void UploadTexture(const void* textureData, uint32_t width, uint32_t height, uint32_t channels, bool generateMipmaps);

		static Renderer Create(const BuilderInfo& info);
		static void Destroy(Renderer& renderer);
	};
}
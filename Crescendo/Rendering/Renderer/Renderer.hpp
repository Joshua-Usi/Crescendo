#pragma once

#include "Core/common.hpp"
#include "Rendering/Reflection/Reflection.hpp"

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
		struct Mesh
		{
			std::vector<float> vertices, normals, textureUVs;
			std::vector<uint32_t> indices;
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
			struct ShaderStageData
			{
				std::vector<uint8_t> vertex, fragment;
				SpirvReflection vertexReflection, fragmentReflection;
				inline ShaderStageData(std::vector<uint8_t> vertex, std::vector<uint8_t> fragment) :
					vertex(vertex), fragment(fragment),
					vertexReflection(ReflectSpirv(vertex)), fragmentReflection(ReflectSpirv(fragment)) {}
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
			// Number of frames in flight, 1 command pool and one command buffer is allocated per frame in flight
			uint32_t framesInFlight;
			/*
			 *	Specifies the number of triangles the vertex buffers can store without resizing
			 *	3 vertices per triangle
			 *	9 4-byte data types per vertex = 36 bytes per vertex
			 *	36 * 3 = 108 bytes per triangle
			 *	Allocates 108 * triangleBufferSize bytes total spread accross 4 different buffers
			 */
			uint32_t triangleBufferSize;

			std::vector<ShaderStageData> shaderData;
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

		void BeginFrame(float r, float g, float b, float a);
		void EndFrame();
		void BindPipeline(uint32_t pipelineIndex);
		template<typename T>
		void UpdatePushConstant(ShaderStage stage, const T& data) { this->UpdatePushConstant(stage, &data, sizeof(data)); }
		void UpdatePushConstant(ShaderStage stage, const void* data, size_t size);
		void Draw(uint32_t mesh);
		void PresentFrame();

		void Resize(const BuilderInfo::WindowExtent& extent);

		void UploadMesh(const Mesh& mesh);

		static Renderer Create(const BuilderInfo& info);
		static void Destroy(Renderer& renderer);
	};
}
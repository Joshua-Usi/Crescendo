#pragma once

#include "common.hpp"
#include "Engine/LayerStack/LayerStack.hpp"
#include "Engine/interfaces/Window.hpp"
#include "Engine/ECS/ECS.hpp"
#include "cs_std/task_queue.hpp"
#include "cs_std/timestamp.hpp"
#include "Engine/Scene/Scene.hpp" 
#include "Rendering/Vulkan/Instance.hpp"
#include "Rendering/Vulkan/ResourceManager.hpp"
#include "Rendering/Vulkan/FrameManager.hpp"

#include "ApplicationCommandLineArgs.hpp"

CS_NAMESPACE_BEGIN
{
	class Font
	{
	public:
		struct Character
		{
			struct ShaderRepresentation
			{
				uint32_t texture;
				float width, height;
				float bearingX, bearingY;
				float advance;
				uint32_t dummy1, dummy2;
			};
			Vulkan::TextureHandle texture;
			float width, height;
			float bearingX, bearingY;
			float advance;
			uint32_t dummy1 = 0, dummy2 = 0; // pad to 32 bytes, can be used later

			ShaderRepresentation GetShaderRepresentation() const { return { texture.GetIndex(), width, height, bearingX, bearingY, advance, 0, 0}; }
		};
	public:
		float ascent, descent, lineGap, lineHeight;
		std::vector<Character> characters;
		Vulkan::BufferHandle characterDataBufferHandle;

		// 95 printable ASCII characters
		Font() : characters(95) {}

		std::vector<Character::ShaderRepresentation> GetShaderRepresentation() const
		{
			std::vector<Character::ShaderRepresentation> data;
			data.reserve(characters.size());
			for (const Character& c : characters)
				data.push_back(c.GetShaderRepresentation());
			return data;
		}
		std::vector<float> GenerateCumulativeAdvance(const std::string& text, uint32_t start = 0, uint32_t count = UINT32_MAX) const
		{
			count = (count > text.size()) ? text.size() : count;
			std::vector<float> data;
			data.reserve(count);
			data.push_back(0.0f);
			// Reserve early, if there are spaces, it's likely we won't use all of the space
			float cumulativeAdvance = 0.0f;
			for (size_t i = start; i < count - 1; i++)
			{
				const char c = text[i];
				if (c < 32 || c > 126) continue;
				if (c == 32)
				{
					cumulativeAdvance += characters[0].advance;
					data.back() = cumulativeAdvance;
					continue;
				}
				cumulativeAdvance += characters[c - 32].advance;
				data.push_back(cumulativeAdvance);
			}
			return data;
		}
	};

	class Application
	{
		friend class Scene;
		friend class LayerUpdate;
	private:
		static Application* self;
		std::vector<std::unique_ptr<Window>> windows;
		LayerStack layerManager;
		cs_std::timestamp timestamp;
		bool isRunning, shouldRestart;

		std::vector<Scene> loadedScenes;
		Scene* activeScene;
	private:
		Vulkan::Instance instance;
	public:
		Vulkan::ResourceManager resourceManager;
	private:
		Vulkan::FrameManager frameManager;

		Vulkan::Vk::Pipeline postProcessingPipeline;
		Vulkan::Vk::Sampler postProcessingSampler;

		Vulkan::Vk::RenderPass depthRenderPass;
		Vulkan::Vk::Framebuffer depthFramebuffer;
		Vulkan::TextureHandle depthImageHandle;
		Vulkan::Vk::Pipeline depthPipeline;

		Vulkan::Vk::RenderPass mainRenderPass;
		Vulkan::Vk::Pipeline mainPipeline;
		Vulkan::TextureHandle mainImageHandle;
		Vulkan::Vk::Framebuffer mainFramebuffer;

		std::vector<Vulkan::BufferHandle> transformsHandle;
		Vulkan::TextureHandle bufferHandle;

		Vulkan::Vk::Pipeline particlePipeline;
		std::vector<Vulkan::BufferHandle> particleBufferHandle;

		std::vector<Vulkan::BufferHandle> directionalLightsHandle;
		std::vector<Vulkan::BufferHandle> pointLightsHandle;
		std::vector<Vulkan::BufferHandle> spotLightsHandle;

		Vulkan::Vk::Pipeline skyboxPipeline;
		Vulkan::MeshHandle skyboxMeshHandle;

		Vulkan::Vk::Pipeline textPipeline;
		std::unordered_map<std::string, Font> fonts;
		std::vector<Vulkan::BufferHandle> textAdvanceDataHandle;
		// Each individual character
		std::vector<Vulkan::BufferHandle> textCharacterDataHandle;

		uint32_t frameIdx = 0;
	protected:
		cs_std::task_queue taskQueue;
	private:
		void InternalUpdate(double dt);
	public:
		Application(const ApplicationCommandLineArgs& args);
		virtual ~Application();
		void Run();
		void Exit();
		void Restart();
		bool IsRunning() const;
		bool ShouldRestart() const;
		template<typename return_type = double> return_type GetTime() { return this->timestamp.elapsed<return_type>(); }
		Window* GetWindow(size_t index = 0) const;
		size_t GetWindowCount() const;
		size_t CreateWindow(const Window::Specification& info);
		void CreateDefaultWindow();
		void CloseWindow(size_t index);
		Scene& GetActiveScene();

		virtual void OnUpdate(double dt) {};
		virtual void OnStartup() {};
		virtual void OnExit() {};

		static Application* Get();
	};
	// To be defined by the client to implement their own inherited Application class
	std::unique_ptr<Application> CreateApplication(const ApplicationCommandLineArgs& args);
}
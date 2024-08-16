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
		Vulkan::Instance instance;
		Vulkan::ResourceManager resourceManager;
		Vulkan::FrameManager frameManager;
	private:
		Vulkan::Vk::Pipeline postProcessingPipeline;
		Vulkan::Vk::Sampler postProcessingSampler;

		Vulkan::Vk::RenderPass depthRenderPass;
		Vulkan::Vk::Framebuffer depthFramebuffer;
		Vulkan::Vk::Image depthImage;
		Vulkan::Vk::Pipeline depthPipeline;

		Vulkan::Vk::RenderPass mainRenderPass;
		Vulkan::Vk::Pipeline mainPipeline;
		Vulkan::TextureHandle mainImageHandle;
		Vulkan::Vk::Framebuffer mainFramebuffer;

		std::vector<Vulkan::BufferHandle> transformsHandle;
		Vulkan::TextureHandle bufferHandle;

		std::vector<Vulkan::BufferHandle> directionalLightsHandle;
		std::vector<Vulkan::BufferHandle> pointLightsHandle;
		std::vector<Vulkan::BufferHandle> spotLightsHandle;

		Vulkan::Vk::Pipeline skyboxPipeline;
		Vulkan::MeshHandle skyboxMeshHandle;
		Vulkan::TextureHandle skyboxTextureHandle;

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
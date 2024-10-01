#pragma once
#include "common.hpp"
#include "Rendering/Vulkan/FrameManager.hpp"
#include "LayerStack.hpp"
#include "Interfaces/Window.hpp"
#include "ECS/ECS.hpp"
#include "Scene/Scene.hpp" 
#include "Rendering/Vulkan/Instance.hpp"
#include "Rendering/Vulkan/FrameManager.hpp"
#include "Rendering/Font.hpp"
#include "cs_std/task_queue.hpp"
#include "cs_std/timestamp.hpp"
#include "ApplicationCommandLineArgs.hpp"
#include "Rendering/RenderResourceManager.hpp"

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
	private:
		Vulkan::Instance instance;
	public:
		RenderResourceManager resourceManager;

		std::vector<Vulkan::Mesh> loadedMeshes;
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
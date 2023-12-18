#pragma once

#include "common.hpp"

#include "Engine/LayerStack/LayerStack.hpp"
#include "Engine/interfaces/Window.hpp"

#include "cs_std/task_queue.hpp"
#include "cs_std/timestamp.hpp"

#include "Rendering/VulkanInstance/VulkanInstance.hpp"

CS_NAMESPACE_BEGIN
{
	class Application
	{
	private:
		static Application* self;

		std::unique_ptr<Window> window;

		LayerStack layerManager;
		cs_std::timestamp timestamp;

		bool isRunning, shouldRestart;
	protected:
		VulkanInstance renderer;
		cs_std::task_queue taskQueue;
	public:
		Application();
		virtual ~Application() = default;
		void Run();
		void Exit()
		{
			this->window->Close();
			this->isRunning = false;
			this->shouldRestart = false;
		};
		void Restart() {
			this->window->Close();
			this->isRunning = false;
			this->shouldRestart = true;
			self = nullptr;
		};
		bool IsRunning() const { return this->isRunning; };
		bool ShouldRestart() const { return this->shouldRestart; };
		template<typename return_type = double> return_type GetTime() { return this->timestamp.elapsed<return_type>(); }
		Window* GetWindow() const { return this->window.get(); };
	public:
		virtual void OnUpdate(double dt) {};
		virtual void OnStartup() {};
		virtual void OnExit() {};
	public:
		static Application* Get() { return self; }
	};
	/// <summary>
	/// To be defined by the client to implement their own inherited Application class
	/// </summary>
	std::unique_ptr<Application> CreateApplication();
}
#pragma once

#include "Core/common.hpp"
#include "Core/LayerStack/LayerStack.hpp"

#include "cs_std/task_queue.hpp"
#include "cs_std/time_manager.hpp"

#include "Engine/interfaces/Window.hpp"
#include "Engine/interfaces/Window.hpp"

namespace Crescendo::Engine
{
	class Application
	{
	private:
		static Application* self;

		shared<Window> window;

		LayerStack layerManager;
		cs_std::time_manager timeManager;

		bool isRunning, shouldRestart;
	protected:
		cs_std::task_queue taskQueue;
	public:
		Application();
		virtual ~Application() = default;
		void Run();
		/// <summary>
		/// Close the application
		/// </summary>
		void Exit()
		{
			this->window->Close();
			this->isRunning = false;
			this->shouldRestart = false;
		};
		/// <summary>
		/// Close the application, then signal that it should restart
		/// </summary>
		void Restart() {
			this->window->Close();
			this->isRunning = false;
			this->shouldRestart = true;
			self = nullptr;
		};
		/// <summary>
		/// Returns the current running state of the application
		/// </summary>
		/// <returns>Really?</returns>
		bool IsRunning() const { return this->isRunning; };
		/// <summary>
		/// Returns if the current application should restart
		/// </summary>
		/// <returns>Really?</returns>
		bool ShouldRestart() const { return this->shouldRestart; };
		/// <summary>
		/// Returns the time since the application has started up. Nanosecond precision
		/// </summary>
		/// <returns>Time (in seconds) since the application started</returns>
		template<typename return_type = double>
		return_type GetTime() { return this->timeManager.now<return_type>(); }
		/// <summary>
		/// Returns the window interface for the current application
		/// </summary>
		/// <returns>Window interface reference</returns>
		shared<Window> GetWindow() const { return this->window; };
		/// <summary>
		/// Runs 60 times every second (60Hz)
		/// </summary>
		virtual void OnUpdate(double dt) {};
		/// <summary>
		/// Runs once when the application starts up, useful for initialisations
		/// </summary>
		virtual void OnStartup() {};
		/// <summary>
		/// Runs once when the application is about to exit, useful for destroying objects, handles or contexts
		/// </summary>
		virtual void OnExit() {};
		/// <summary>
		/// Get a reference to the singleton application class
		/// </summary>
		/// <returns>reference to the current application</returns>
		static Application* Get() { return self; }
	};
	/// <summary>
	/// To be defined by the client to implement their own inherited Application class
	/// </summary>
	unique<Application> CreateApplication();
}
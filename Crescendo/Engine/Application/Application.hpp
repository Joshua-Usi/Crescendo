#pragma once

#include "Core/common.hpp"
#include "Core/LayerStack/LayerStack.hpp"

#include "cs_std/task_queue.hpp"
#include "cs_std/time_manager.hpp"

#include "Engine/interfaces/Window.hpp"

#include "Rendering/Atlas/Atlas.hpp"

namespace Crescendo::Engine
{
	class Application
	{
	private:
		shared<Window> window;

		LayerStack layerManager;
		cs_std::time_manager timeManager;

		static Application* self;
	protected:
		cs_std::task_queue taskQueue;
		Atlas renderer;
	public:
		Application();
		virtual ~Application() = default;
		void Run();
		/// <summary>
		/// Close the application
		/// </summary>
		inline void Exit() { this->window->Close(); };
		/// <summary>
		/// Returns the current running state of the application. This is usually tied to the state of the window
		/// </summary>
		/// <returns>Really?</returns>
		inline bool IsRunning() const { return this->window->IsOpen(); };
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
		inline shared<Window> GetWindow() const { return this->window; };
		/// <summary>
		/// Runs 60 times every second (60Hz)
		/// </summary>
		inline virtual void OnUpdate(double dt) {};
		/// <summary>
		/// Runs once when the application starts up, useful for initialisations
		/// </summary>
		inline virtual void OnStartup() {};
		/// <summary>
		/// Runs once when the application is about to exit, useful for destroying objects, handles or contexts
		/// </summary>
		inline virtual void OnExit() {};
		/// <summary>
		/// Get a reference to the singleton application class
		/// </summary>
		/// <returns>reference to the current application</returns>
		inline static Application* Get() { return self; }
	};
	/// <summary>
	/// To be defined by the client to implement their own inherited Application class
	/// </summary>
	unique<Application> CreateApplication();
}
#pragma once

#include "Core/common.hpp"
#include "Core/LayerStack/LayerStack.hpp"
#include "Core/include/TimeManager.hpp"

#include "Engine/interfaces/Window.hpp"

namespace Crescendo::Engine
{
	class Application
	{
	private:
		shared<Window> window;

		Core::LayerStack layerManager;
		Core::TimeManager timeManager;

		static Application* self;
	public:
		Application();
		virtual ~Application() = default;
		void Run();
		/// <summary>
		/// Close the application
		/// </summary>
		void Exit();
		/// <summary>
		/// Returns the current running state of the application. This is usually tied to the state of the window
		/// </summary>
		/// <returns>Really?</returns>
		bool IsRunning() const;
		/// <summary>
		/// Returns the time since the application has started up. Nanosecond precision
		/// </summary>
		/// <returns>Time (in seconds) since the application started</returns>
		double GetTime();
		/// <summary>
		/// Returns the window interface for the current application
		/// </summary>
		/// <returns>Window interface reference</returns>
		shared<Window> GetWindow() const;
		/// <summary>
		/// Runs 60 times every second (60Hz)
		/// </summary>
		virtual void OnUpdate(double dt);
		/// <summary>
		/// Runs once when the application starts up, useful for initialisations
		/// </summary>
		virtual void OnStartup();
		/// <summary>
		/// Runs once when the application is about to exit, useful for destroying objects, handles or contexts
		/// </summary>
		virtual void OnExit();
		/// <summary>
		/// Get a reference to the singleton application class
		/// </summary>
		/// <returns>reference to the current application</returns>
		static Application* Get();
	};
	/// <summary>
	/// To be defined by the client to implement their own inherited Application class
	/// </summary>
	shared<Application> CreateApplication();
}
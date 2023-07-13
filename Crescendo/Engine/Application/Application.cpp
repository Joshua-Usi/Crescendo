#include "Application.hpp"

#include "Engine/layers/Update.hpp"

namespace Crescendo::Engine
{
	// Assign self and null as no instance exists yet
	Application* Application::self = nullptr;
	Application::Application()
	{
		CS_ASSERT(self == nullptr, "Application instance already exists!");
		self = this;
		// Create window
		this->window = Window::Create();

		// Attach layers
		uint32_t refreshRate = this->window->GetRefreshRate();
		double secondsPerFrame = (refreshRate == 0) ? 0.0 : 1.0 / double(refreshRate);
		this->layerManager.Attach(new Core::LayerUpdate(secondsPerFrame, 0));

		// Initialise layers at current time
		this->layerManager.Init(this->timeManager.GetTimeSinceStart<double>());
	}
	void Application::Run()
	{
		CS_TIME(this->OnStartup(), "Startup");

		while (this->IsRunning())
		{
			double time = this->timeManager.GetTimeSinceStart<double>();
			// I'm going to make a safe presumption that the time for query is a very minute amount of time
			// So we don't need to the time again, tests have shown it is in the 100's of nanoseconds
			this->layerManager.QueryForUpdate(time);
			this->layerManager.Update(time);
		}

		CS_TIME(this->OnExit(), "Exit");
	}
	void Application::Exit() { this->window->Close(); }
	bool Application::IsRunning() const { return this->window->IsOpen(); }

	// User-space functions
	void Application::OnStartup() {}
	void Application::OnUpdate(double dt) {}
	void Application::OnExit() {}

	// Getters
	Application* Application::Get() { return self; }
	double Application::GetTime() { return this->timeManager.GetTimeSinceStart<double>(); }
	shared<Window> Application::GetWindow() const { return this->window; }
}
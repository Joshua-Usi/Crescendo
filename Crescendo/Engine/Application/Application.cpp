#include "Application.hpp"

#include "Engine/layers/Update.hpp"

namespace Crescendo::Engine
{
	// Assign self and null as no instance exists yet
	Application* Application::self = nullptr;
	Application::Application()
		: taskQueue(TaskQueue()), timeManager(TimeManager()), layerManager(LayerStack()), window(Window::Create())
	{
		CS_ASSERT(self == nullptr, "Application instance already exists!");
		self = this;
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
			/*
			 *	I'm going to make a safe presumption that the time for query is a very minute amount of time
			 *	So we don't need to the time again, tests have shown it is in the 100's of nanoseconds
			 *	Pretty much doesn't matter unless you want to run an update at least 10 million times a second
			 *	Of which there is much better ways of doing so
			 */
			this->layerManager.QueryForUpdate(time);
			this->layerManager.Update(time);
		}
		CS_TIME(this->OnExit(), "Exit");
	}
}
#include "Application.hpp"

#include "Engine/layers/Update.hpp"
#include "Engine/CVar/Cvar.hpp"

CS_NAMESPACE_BEGIN
{
	// Assign self and null as no instance exists yet
	Application* Application::self = nullptr;
	Application::Application()
		: isRunning(true), shouldRestart(false), taskQueue(cs_std::task_queue()), timeManager(cs_std::time_manager()), layerManager(LayerStack())
	{
		CS_ASSERT(self == nullptr, "Application instance already exists!");
		self = this;

		CVar::LoadConfigXML("config.xml");

		window = Window::Create({
			CVar::Get<std::string>("ec_appname"),
			CVar::Get<uint32_t>("ec_windowwidth"),
			CVar::Get<uint32_t>("ec_windowheight"),
		});

		// Attach layers
		const uint32_t refreshRate = this->window->GetRefreshRate();
		const double secondsPerFrame = (refreshRate == 0 || !CVar::Get<bool>("ec_vsync")) ? 0.0 : 1.0 / double(refreshRate);
		this->layerManager.Attach(new LayerUpdate(secondsPerFrame));
		this->layerManager.Init(this->timeManager.now<double>());
	}
	void Application::Run()
	{
		this->OnStartup();
		while (this->IsRunning())
		{
			const double time = this->timeManager.now<double>();
			this->layerManager.QueryForUpdate(time);
			this->layerManager.Update(time);
		}
		this->OnExit();
	}
}
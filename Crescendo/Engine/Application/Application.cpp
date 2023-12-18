#include "Application.hpp"

#include "Engine/layers/Update.hpp"
#include "Engine/CVar/Cvar.hpp"

CS_NAMESPACE_BEGIN
{
	// Assign self and null as no instance exists yet
	Application* Application::self = nullptr;
	Application::Application()
		: isRunning(true), shouldRestart(false), taskQueue(cs_std::task_queue()), timestamp(), layerManager(LayerStack())
	{
		self = this;
		CVar::LoadConfigXML("config.xml");

		window = Window::Create({
			CVar::Get<std::string>("ec_appname"),
			CVar::Get<uint32_t>("ec_windowwidth"),
			CVar::Get<uint32_t>("ec_windowheight"),
		});

		const uint32_t refreshRate = this->window->GetRefreshRate();
		const double secondsPerFrame = (refreshRate == 0 || !CVar::Get<bool>("ec_vsync")) ? 0.0 : 1.0 / double(refreshRate);
		this->layerManager.Attach(new LayerUpdate(secondsPerFrame));
		this->layerManager.Init(this->timestamp.elapsed<double>());

		this->renderer = VulkanInstance({
			.enableValidationLayers = CVar::Get<bool>("irc_validationlayers"),
			.appName = CVar::Get<std::string>("ec_appname"),
			.engineName = "Crescendo",
			.window = this->GetWindow()->GetNative(),
			.descriptorSetsPerPool = CVar::Get<uint32_t>("irc_descriptorsetsperpool"),
			.framesInFlight = CVar::Get<uint32_t>("rc_framesinflight"),
			.anisotropicSamples = CVar::Get<uint32_t>("rc_anisotropicsamples"),
			.multisamples = CVar::Get<uint32_t>("rc_multisamples"),
			.renderScale = CVar::Get<float>("rc_renderscale")
		});
	}
	void Application::Run()
	{
		this->OnStartup();
		while (this->IsRunning())
		{
			const double time = this->timestamp.elapsed<double>();
			this->layerManager.QueryForUpdate(time);
			this->layerManager.Update(time);
		}
		this->OnExit();
	}
}
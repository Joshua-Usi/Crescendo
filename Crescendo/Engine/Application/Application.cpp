#include "Application.hpp"

#include "Engine/layers/Update.hpp"
#include "Engine/CVar/Cvar.hpp"

namespace Crescendo::Engine
{
	// Assign self and null as no instance exists yet
	Application* Application::self = nullptr;
	Application::Application()
		: taskQueue(cs_std::task_queue()), timeManager(cs_std::time_manager()), layerManager(LayerStack()), window(Window::Create())
	{
		CS_ASSERT(self == nullptr, "Application instance already exists!");
		self = this;

		CVar::LoadConfigXML("config.xml");

		// Attach layers
		uint32_t refreshRate = this->window->GetRefreshRate();
		double secondsPerFrame = (refreshRate == 0 || !CVar::Get<bool>("ec_vsync")) ? 0.0 : 1.0 / double(refreshRate);
		this->layerManager.Attach(new LayerUpdate(secondsPerFrame, 0));
		this->layerManager.Init(this->timeManager.now<double>());

		this->window->SetSize(CVar::Get<int64_t>("ec_windowwidth"), CVar::Get<int64_t>("ec_windowheight"));

		Renderer::BuilderInfo info = {};

		info.useValidationLayers = CVar::Get<bool>("rc_validationlayers");
		info.appName = CVar::Get<std::string>("rc_appname");
		info.engineName = CVar::Get<std::string>("rc_enginename");
		info.window = this->window->GetNative();
		info.windowExtent = { this->window->GetWidth(), this->window->GetHeight() };
		info.preferredPresentMode = Renderer::BuilderInfo::PresentMode::Mailbox;
		info.framesInFlight = CVar::Get<int64_t>("rc_framesinflight");
		info.descriptorBufferBlockSize = CVar::Get<int64_t>("rc_descriptorbufferblocksize");
		info.msaaSamples = CVar::Get<int64_t>("rc_multisamples");
		info.anistropicFiltering = CVar::Get<float>("rc_anisotropicfiltering");
		info.desriptorSetsPerPool = CVar::Get<int64_t>("rc_descriptorsetsperpool");
		info.shadowMapResolution = CVar::Get<int64_t>("rc_shadowmapresolution");

		this->renderer = Atlas(info);
	}
	void Application::Run()
	{
		this->OnStartup();
		while (this->IsRunning())
		{
			double time = this->timeManager.now<double>();
			/*
			 *	I'm going to make a safe presumption that the time for query is a very minute amount of time
			 *	So we don't need to the time again, tests have shown it is in the 100's of nanoseconds
			 *	Pretty much doesn't matter unless you want to run an update at least 10 million times a second
			 *	Of which there is much better ways of doing so
			 */
			this->layerManager.QueryForUpdate(time);
			this->layerManager.Update(time);
		}
		this->OnExit();
	}
}
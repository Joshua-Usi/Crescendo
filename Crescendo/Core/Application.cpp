#include "Application.hpp"
#include "Layers/Update.hpp"
#include "Static/Cvar.hpp"
#include "cs_std/file.hpp"
#include "Rendering/Vulkan/Create.hpp"
#include "Libraries/Construct/Construct.hpp"
#include "Assets/ImageLoaders.hpp"
#include "utils/utils.hpp"

CS_NAMESPACE_BEGIN
{
	static bool isFirstWindow = true;
	Application* Application::self = nullptr;
	Application::Application(const ApplicationCommandLineArgs& args) : isRunning(true), shouldRestart(false)
	{
		self = this;
		// Either use the default config, which should be in the same directory as the executable, named "config.xml" or can be specified in the command line
		std::string config = args.HasArg("config") ? args.GetArg("config") : "config.xml";
		if (cs_std::text_file(config).exists())
			CVar::LoadConfigXML(config);

		this->CreateDefaultWindow();

		renderer.Init();

		// Create a default scene
		loadedScenes.emplace_back();
		activeScene = &loadedScenes[0];
	}
	Application::~Application()
	{
		renderer.WaitIdle();
	}
	void Application::InternalUpdate(double dt)
	{
		// Active scene
		Scene& currentScene = *activeScene;

		// Update entity behaviours
		currentScene.entityManager.ForEach<Behaviours>([&](Behaviours& b) {
			b.OnUpdate(dt);
		});
		// Update particle emitters
		currentScene.entityManager.ForEach<ParticleEmitter>([&](ParticleEmitter& emitter) {
			emitter.Update(GetTime<float>(), dt);
		});

		renderer.RenderScene(*activeScene);
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
	void Application::Exit()
	{
		for (auto& window : this->windows)
			window->Close();
		this->isRunning = false;
		this->shouldRestart = false;
	}
	void Application::Restart()
	{
		for (auto& window : this->windows)
			window->Close();
		this->isRunning = false;
		this->shouldRestart = true;
		isFirstWindow = true;
		self = nullptr;
	}
	bool Application::IsRunning() const { return this->isRunning; };
	bool Application::ShouldRestart() const { return this->shouldRestart; };
	Window* Application::GetWindow(size_t index) const { return this->windows[index].get(); };
	size_t Application::GetWindowCount() const { return this->windows.size(); };
	size_t Application::CreateWindow(const Window::Specification& info)
	{
		this->windows.push_back(Window::Create(info));

		// If this is the first window, attach the update layer
		if (isFirstWindow)
		{
			uint32_t refreshRate = 0;

			// -1 for unlimited, 0 for vsync, >0 for fixed refresh rate
			int32_t expectedRefreshRate = CVar::Get<int32_t>("ec_refreshrate");

			if (expectedRefreshRate < 0)
				refreshRate = 0;
			else if (expectedRefreshRate == 0)
				refreshRate = this->windows[0]->GetRefreshRate();
			else
				refreshRate = expectedRefreshRate;

			const double secondsPerFrame = (refreshRate == 0) ? 0.0 : 1.0 / double(refreshRate);
			this->layerManager.Attach(new LayerUpdate(secondsPerFrame));
			this->layerManager.Init(this->timestamp.elapsed<double>());
			isFirstWindow = false;
		}

		// Return the index of the window
		return this->windows.size() - 1;
	};
	void Application::CreateDefaultWindow()
	{
		if (this->windows.size() > 0)
		{
			cs_std::console::warn("Attempted to create a default window when one already exists");
			return;
		}

		// Override if it's going to be fullscreen
		bool isNotFullscreen = !CVar::Get<bool>("ec_fullscreen");

		this->CreateWindow(Window::Specification(
			CVar::Get<std::string>("ec_appname"),
			CVar::Get<uint32_t>("ec_windowwidth") * isNotFullscreen,
			CVar::Get<uint32_t>("ec_windowheight") * isNotFullscreen
		));
	}
	void Application::CloseWindow(size_t index)
	{
		this->windows[index]->Close();
		this->windows.erase(this->windows.begin() + index);
	}
	Scene& Application::GetActiveScene() { return *activeScene; }
	Application* Application::Get() { return self; }
}
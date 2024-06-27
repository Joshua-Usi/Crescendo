#include "Application.hpp"

#include "Engine/layers/Update.hpp"
#include "Engine/CVar/Cvar.hpp"

#include "Rendering/Vulkan2/CommandPass/DepthPrePass.hpp"
#include "Rendering/Vulkan2/CommandPass/PostProcessingPass.hpp"

CS_NAMESPACE_BEGIN
{
	static bool isFirstWindow = true;
	// Assign self and null as no instance exists yet
	Application* Application::self = nullptr;
	Application::Application()
		: isRunning(true), shouldRestart(false), taskQueue(cs_std::task_queue()), timestamp(), layerManager(LayerStack())
	{
		self = this;
		CVar::LoadConfigXML("./configs/config.xml");

		this->CreateDefaultWindow();
		this->GetWindow()->SetCursorLock(true);

		Vulkan::InstanceSpecification spec{
			CVar::Get<bool>("irc_validationlayers"),
			CVar::Get<std::string>("ec_appname"), "Crescendo",
		};
		this->instance = Vulkan::Instance(spec);
		this->instance.CreateSurface(this->GetWindow()->GetNative(), {
			.descriptorManagerSpec = {
				CVar::Get<uint32_t>("irc_maxbufferdescriptors_bindless"),
				CVar::Get<uint32_t>("irc_maxtexturedescriptors_bindless")
			},
			.swapchainRecreationCallback = nullptr
		});
		this->frameManager = Vulkan::FrameManager(this->instance.GetSurface(0).GetDevice(), CVar::Get<uint32_t>("rc_framesinflight"));
		this->resourceManager = Vulkan::ResourceManager(this->instance.GetSurface(0).GetDevice());
		this->passes.push_back(new DepthPrePass(this->instance.GetSurface(0), CVar::Get<std::string>("ircs_depthprepass"), VK_FORMAT_D32_SFLOAT, VK_SAMPLE_COUNT_1_BIT));
		this->passes.push_back(new PostProcessPass(this->instance.GetSurface(0), CVar::Get<std::string>("ircs_postprocessing")));

		// Create a default scene
		loadedScenes.emplace_back();
		activeScene = &loadedScenes[0];
	}
	Application::~Application()
	{
		this->instance.GetSurface(0).GetDevice().WaitIdle();
		for (Vulkan::CommandPass* pass : passes) delete pass;
	}
	void Application::InternalUpdate(double dt)
	{
		Vulkan::Surface& surface = this->instance.GetSurface(0);
		Vulkan::FrameResources& frame = this->frameManager.GetCurrentFrame();
		Vulkan::Vk::GraphicsCommandQueue& cmd = frame.commandBuffer;
		Scene& currentScene = *activeScene;

		currentScene.entityManager.ForEach<Behaviours>([&](Behaviours& b) {
			b.OnUpdate(dt);
		});

		std::vector<DirectionalLight::ShaderRepresentation> directionalLights(16);
		std::vector<PointLight::ShaderRepresentation> pointLights(64);
		std::vector<SpotLight::ShaderRepresentation> spotLights(64);

		currentScene.entityManager.ForEach<Transform, DirectionalLight>([&](Transform& transform, DirectionalLight& light) {
			directionalLights.push_back(light.CreateShaderRepresentation(transform));
		});
		currentScene.entityManager.ForEach<Transform, PointLight>([&](Transform& transform, PointLight& light) {
			pointLights.push_back(light.CreateShaderRepresentation(transform));
		});
		currentScene.entityManager.ForEach<Transform, SpotLight>([&](Transform& transform, SpotLight& light) {
			spotLights.push_back(light.CreateShaderRepresentation(transform));
		});

		cmd.WaitCompletion();
		cmd.Reset();

		surface.AcquireNextImage(frame.imageAvailable);

		cmd.Begin();
		for (Vulkan::CommandPass* pass : passes) pass->Execute(cmd);
		cmd.End();
		cmd.Submit(frame.imageAvailable, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, frame.renderFinished);

		surface.Present(cmd.GetQueue(), frame.renderFinished);

		this->frameManager.AdvanceFrame();
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
		for (auto& window : this->windows) window->Close();
		this->isRunning = false;
		this->shouldRestart = false;
	}
	void Application::Restart()
	{
		for (auto& window : this->windows) window->Close();
		this->isRunning = false;
		this->shouldRestart = true;
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
			const uint32_t refreshRate = this->windows[0]->GetRefreshRate();
			const double secondsPerFrame = (refreshRate == 0 || !CVar::Get<bool>("ec_vsync")) ? 0.0 : 1.0 / double(refreshRate);
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
		this->CreateWindow(Window::Specification(
			CVar::Get<std::string>("ec_appname"),
			CVar::Get<uint32_t>("ec_windowwidth"),
			CVar::Get<uint32_t>("ec_windowheight")
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
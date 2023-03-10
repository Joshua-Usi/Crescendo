#include "Application.h"

#include "engineLayers/LayerUpdate.h"
#include "engineLayers/LayerRender.h"
#include "engineLayers/LayerUI.h"
#include "engineLayers/LayerRenderLate.h"

namespace Crescendo
{
	namespace Engine
	{
		Application* Application::self = nullptr;
		Application::Application()
		{
			window = Window::Create();

			CS_ASSERT(self == nullptr, "Application instance already exists!");
			this->self = this;

			uint32_t refreshRate = this->window->GetRefreshRate();
			double secondsPerFrame = (refreshRate == 0) ? 0 : 1.0 / double(refreshRate);
			Console::EngineLog("Running at {}fps", refreshRate);

			// At refresh rate speeds
			this->layerManager.Attach(new LayerUpdate(secondsPerFrame, 0));
			this->layerManager.Attach(new LayerRender(secondsPerFrame, 1000));
			this->layerManager.Attach(new LayerUI(secondsPerFrame, 3000));
			this->layerManager.Attach(new LayerRenderLate(secondsPerFrame, 4000));

			// Initialise layers at current time
			this->layerManager.Init(this->timeManager.GetTimeSinceStart<double>());
		}
		Application::~Application()
		{
			delete window;
		}
		void Application::Run()
		{
			this->OnStartup();
			while (this->IsRunning())
			{
				double time = this->timeManager.GetTimeSinceStart<double>();
				this->layerManager.QueryForUpdate(time);
				time = this->timeManager.GetTimeSinceStart<double>();
				this->layerManager.Update(time);
			}
			this->OnExit();
		}
		void Application::Close()
		{
			this->GetWindow()->Close();
		}
		bool Application::IsRunning()
		{
			return this->GetWindow()->IsOpen();
		}
		void Application::OnStartup()
		{
			
		}
		void Application::OnUpdate(double dt)
		{

		}
		void Application::OnExit()
		{
			glfwTerminate();
		}
		Application* Application::Get()
		{
			return self;
		}
		double Application::GetTime()
		{
			return this->timeManager.GetTimeSinceStart<double>();
		}
		Window* Application::GetWindow() {
			return this->window;
		}
	}
}
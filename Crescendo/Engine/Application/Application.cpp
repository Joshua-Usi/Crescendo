#include "Application.h"

#include "engineLayers/LayerUpdate.h"
#include "engineLayers/LayerRender.h"
#include "engineLayers/LayerUI.h"

namespace Crescendo
{
	namespace Engine
	{
		Application* Application::self = nullptr;
		Application::Application()
		{
			window = Window::Create();

			self = this;

			// 60Hz updates
			Layer* update = new LayerUpdate(1.0 / 60.0, 0);
			layerManager.Attach(update);

			gt::Int32 refreshRate = window->GetRefreshRate();
			double secondsPerFrame = (refreshRate == 0) ? 0 : 1.0 / double(refreshRate);

			// At refresh rate speeds
			Layer* render = new LayerRender(secondsPerFrame * 0, 1000);
			layerManager.Attach(render);

			Layer* render2 = new LayerUI(secondsPerFrame * 0, 2000);
			layerManager.Attach(render2);

			layerManager.Init(timeManager.GetTimeSinceStart<double>());
		}
		Application::~Application()
		{
			// Delete layers in the order they were added, though its not a
			// very performant algorithm, its fine for this case
			while (layerManager.Count() > 1) {
				delete layerManager.Detach(0);
			}
		}
		void Application::Run()
		{
			OnStartup();
			while (IsRunning())
			{
				double time = timeManager.GetTimeSinceStart<double>();
				layerManager.QueryForUpdate(time);
				layerManager.Update(time);
			}
			OnExit();
		}
		bool Application::IsRunning()
		{
			return GetWindow()->IsOpen();
		}
		void Application::OnStartup()
		{
			
		}
		void Application::OnUpdate()
		{
			
		}
		void Application::OnExit()
		{
			
		}
		Application* Application::Get()
		{
			return self;
		}
		double Application::GetTime()
		{
			return timeManager.GetTimeSinceStart<double>();
		}
		Window* Application::GetWindow() {
			return window;
		}
	}
}
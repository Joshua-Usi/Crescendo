#include "Crescendo.h"	
using namespace Crescendo::Engine;

class Sandbox : public Application
{
	void OnStartup()
	{
		Console::BeginFileLog("Sandbox");
		Console::Log("Startup");
	}
	void OnUpdate()
	{
			
	}
	void OnExit()
	{
		Console::Log("Exit");
		Console::EndFileLog();
	}
};

Application* Crescendo::Engine::CreateApplication() {
	return new Sandbox();
}
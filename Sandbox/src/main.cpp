#include "Crescendo.h"

#include "XML/XML.h"
using namespace Crescendo::Engine;

class Sandbox : public Application
{
	void OnStartup()
	{
		Console::BeginFileLog("Sandbox");
	}
	void OnUpdate()
	{

	}
	void OnExit()
	{
		Console::EndFileLog();
	}
};

Application* Crescendo::Engine::CreateApplication() {
	return new Sandbox();
}
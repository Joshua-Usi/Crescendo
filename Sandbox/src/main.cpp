#include "Crescendo.h"
using namespace Crescendo::Engine;

class Sandbox : public Application
{
	void OnStartup()
	{
		Console::BeginFileLog("Sandbox");
		Console::Log("Startup");

		Crescendo::Tools::XML::XMLDocument doc;
		int err = Crescendo::Tools::XML::ParseFromFile(&doc, "attributes.xml");
		Console::Log("Error Code: {}", err);
	}
	void OnUpdate()
	{
		if (Input::GetKeyPressed(Key::ShiftLeft)) {
			Console::Log("Hello!");
		}
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
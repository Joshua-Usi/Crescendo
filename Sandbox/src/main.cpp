#include "Crescendo.h"

#include "XML/XML.h"
using namespace Crescendo::Engine;

class Sandbox : public Application
{
	void OnStartup()
	{
		Console::BeginFileLog("Sandbox");
		Console::Log("Startup");

		Crescendo::Tools::XML::Document doc;
		Crescendo::Tools::XML::ParseFromFile(&doc, "./xml/part.xml");

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
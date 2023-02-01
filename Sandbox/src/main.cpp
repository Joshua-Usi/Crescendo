#include "Crescendo.h"

#include "XML/XML.h"
using namespace Crescendo::Engine;

class Sandbox : public Application
{
	void OnStartup()
	{
		Console::BeginFileLog("Sandbox");
		Console::Log("Startup");

		double now = GetTime();
		const char* documentPath = "./xml/nasa.xml";
		Crescendo::Tools::XML::Document doc;
		Crescendo::Tools::XML::ParseFromFile(&doc, documentPath);
		Console::Log("{} took {}s to parse", documentPath, GetTime() - now);

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
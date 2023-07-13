#include "Crescendo.hpp"
using namespace Crescendo::Engine;

class Sandbox : public Application
{
public:
	void OnStartup()
	{
		Console::BeginFileLog("Sandbox");
	}
	void OnUpdate(double dt)
	{
		if (Input::GetKeyDown(Key::Escape)) this->Exit();
	}
	void OnExit()
	{
		Console::EndFileLog();
	}
};

RegisterApp(Sandbox);
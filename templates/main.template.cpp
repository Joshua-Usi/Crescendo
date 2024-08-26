#include "Crescendo.hpp"
using namespace CrescendoEngine;

class <project_name> : public Application
{
public:
	using Application::Application;
	void OnStartup()
	{
		// OnStartup is called when the application is started
	}
	void OnUpdate(double dt)
	{
		// OnUpdate is called every frame
	}
	void OnExit()
	{
		// OnExit is called when the application is closed
	}
};

CrescendoRegisterApp(<project_name>);
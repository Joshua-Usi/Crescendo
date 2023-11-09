#pragma once

#include "Engine/Application/Application.hpp"

#ifdef CS_PLATFORM_WINDOWS
// Entrypoint so the user does not need to write their own main function
int main(int argc, char** argv)
{
	// TODO Handle command line arguments
	unique<Crescendo::Engine::Application> app;
	do
	{
		app = Crescendo::Engine::CreateApplication();
		app->Run();
	} while (app->ShouldRestart());
	return 0;
}
#else
	#error Crescendo only supports Windows!
#endif
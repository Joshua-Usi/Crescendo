#pragma once

#include "Engine/Application/Application.hpp"

#ifdef CS_PLATFORM_WINDOWS
// Entrypoint so the user does not need to write their own main function
int main(int argc, char** argv)
{
	// TODO Handle command line arguments

	shared<Crescendo::Engine::Application> app = Crescendo::Engine::CreateApplication();
	app->Run();
	return 0;
}
#else
	#error Crescendo only supports Windows!
#endif
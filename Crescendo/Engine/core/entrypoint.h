#pragma once

#include "Application/Application.h"

#ifdef CS_PLATFORM_WINDOWS
// Entrypoint so the user does not need to write their own main function
int main(int argc, char** argv)
 {
	auto app = Crescendo::Engine::CreateApplication();
	app->Run();
	delete app;
	return 0;
}
	#ifdef CS_PROD
// Entrypoint when console is not needed
int WinMain(int argc, char** argv)
{
	return main(argc, argv);
}
	#endif
#else
	#error Windows is currently only supported
#endif
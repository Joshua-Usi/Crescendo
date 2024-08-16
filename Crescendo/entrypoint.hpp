#pragma once

#include "Engine/Application/ApplicationCommandLineArgs.hpp"
#include "Engine/Application/Application.hpp"

#ifdef CS_PLATFORM_WINDOWS
	int main(int argc, char** argv)
	{
		// Arguments persist through restarts
		CrescendoEngine::ApplicationCommandLineArgs commandLineArgs(argc, argv);
		std::unique_ptr<CrescendoEngine::Application> app;
		do
		{
			app = CrescendoEngine::CreateApplication(commandLineArgs);
			app->Run();
		} while (app->ShouldRestart());
		return 0;
	}
#else
	#error Crescendo only supports Windows!
#endif
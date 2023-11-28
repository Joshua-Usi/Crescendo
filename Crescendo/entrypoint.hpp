#pragma once

#include "Engine/Application/Application.hpp"

#ifdef CS_PLATFORM_WINDOWS
	int main(int argc, char** argv)
	{
		// TODO Handle command line arguments
		std::unique_ptr<CrescendoEngine::Application> app;
		do
		{
			app = CrescendoEngine::CreateApplication();
			app->Run();
		} while (app->ShouldRestart());
		return 0;
	}
#else
	#error Crescendo only supports Windows!
#endif
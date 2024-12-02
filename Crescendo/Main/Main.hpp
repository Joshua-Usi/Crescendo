#pragma once
#include "Interfaces/Module.hpp"

#include <iostream>
#include "Console.hpp"

class Main : public CrescendoEngine::Module
{
public:
	void OnLoad() override
	{

	}
	void OnUnload() override
	{

	}
	void OnUpdate(double dt) override
	{
		//std::cout << "Update..." << std::endl;
		CrescendoEngine::Console::Log("Update...");
	}

	static CrescendoEngine::ModuleMetadata GetMetadata()
	{
		return
		{
			"Main",
			"0.0.1",
			"Main module for Crescendo",
			"Joshua Usi",
			"",
			0.5
		};
	}
};
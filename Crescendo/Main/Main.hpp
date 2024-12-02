#pragma once
#include "Interfaces/Module.hpp"

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

	}

	static CrescendoEngine::ModuleMetadata GetMetadata()
	{
		return
		{
			"Main",
			"0.0.1",
			"Main module for Crescendo",
			"Joshua Usi",
			nullptr,
			0.5
		};
	}
};
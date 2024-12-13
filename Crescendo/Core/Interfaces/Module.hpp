#pragma once
#include "OSDetection.hpp"

namespace CrescendoEngine
{
	struct ModuleMetadata
	{
		const char* name;
		const char* version;
		const char* description;
		const char* author;
		// comma separated list
		const char* dependencies;
		double updateInterval;
	};

	class Module
	{
	public:
		virtual ~Module() {};
		virtual void OnLoad() = 0;
		virtual void OnUnload() = 0;
		virtual void OnUpdate(double dt) = 0;
		// Module authors must implement a static 'GetMetadata' method.
		// This method should return a 'ModuleMetadata' struct.
		// static ModuleMetadata GetMetadata();
	};
}

#define CS_CREATE_MODULE_FACTORY_FUNCTION(mod) extern "C" CS_MODULE_EXPORT CrescendoEngine::Module* CreateModule() { return new mod(); }
#define CS_CREATE_MODULE_METADATA_FUNCTION(mod) extern "C" CS_MODULE_EXPORT CrescendoEngine::ModuleMetadata GetMetadata() { return mod::GetMetadata(); }
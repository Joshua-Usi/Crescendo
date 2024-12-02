#pragma once
#include "Interfaces/Module.hpp"

#include <vector>
#include <memory>
#include <filesystem>
#include <string>

namespace CrescendoEngine
{
	class Core
	{
	private:
		struct ModuleData
		{
			void* dllHandle = nullptr;
			std::unique_ptr<Module> module;
		};

		std::vector<ModuleData> m_loadedModules;

		// Loads a configuration file and returns the entrypoint module
		std::string LoadConfig(const std::filesystem::path& path);
		void LoadModule(const std::filesystem::path& path);
		void LoadModules(const std::vector<std::string>& modules);
		void MainLoop();
		void UnloadModules();
	public:
		void Run(const std::filesystem::path& configPath);
	};
}
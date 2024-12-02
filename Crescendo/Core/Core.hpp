#pragma once
#include "Interfaces/Module.hpp"

#include <vector>
#include <memory>
#include <filesystem>
#include <string>
#include <unordered_set>

namespace CrescendoEngine
{
	using CreateModuleFunc = Module * (*)();
	using GetMetadataFunc = ModuleMetadata(*)();

	class Core
	{
	private:
		struct ModuleData
		{
			void* dllHandle = nullptr;
			std::unique_ptr<Module> module;
		};
		struct ModuleCreateData
		{
			void* dllHandle = nullptr;
			CreateModuleFunc createModule = nullptr;
			GetMetadataFunc getMetadata = nullptr;
		};

		std::vector<ModuleData> m_loadedModules;

		// Loads a configuration file and returns the entrypoint module
		std::string LoadConfig(const std::filesystem::path& path);
		// Loads the entrypoint module and all the dependencies
		void LoadModule(
			const std::filesystem::path& path, std::vector<ModuleCreateData>& modules,
			std::unordered_set<std::string>& loadingModules, std::unordered_set<std::string>& loadedModules
		);
		void InitializeModules(const std::vector<ModuleCreateData>& modules);
		void MainLoop();
		void UnloadModules();
	public:
		void Run(const std::filesystem::path& configPath);
	};
}
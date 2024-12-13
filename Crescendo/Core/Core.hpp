#pragma once
#include "Interfaces/Module.hpp"
#include <vector>
#include <memory>
#include <filesystem>
#include <string>
#include <unordered_set>
#include "ECS/EntityRegistry.hpp"
#include "OSDetection.hpp"

namespace CrescendoEngine
{
	using CreateModuleFunc = Module*(*)();
	using GetMetadataFunc = ModuleMetadata(*)();

	class CS_CORE_EXPORT Core
	{
	private:
		struct ModuleData
		{
			void* dllHandle = nullptr;
			CreateModuleFunc createModule = nullptr;
			GetMetadataFunc getMetadata = nullptr;
			std::unique_ptr<Module> module;
		};
	private:
		static Core* s_instance;
	private:
		std::unordered_map<std::string, ModuleData> m_loadedModules;
		EntityRegistry m_entityRegistry;
	private:
		// Loads a configuration file and returns the entrypoint module
		std::string LoadConfig(const std::filesystem::path& path);
		// Loads the entrypoint module and all the dependencies
		void LoadModule(
			const std::filesystem::path& path, std::vector<ModuleData>& modules,
			std::unordered_set<std::string>& loadingModules, std::unordered_set<std::string>& loadedModules
		);
		void InitializeModules(const std::vector<ModuleData>& modules);
		void MainLoop();
		void UnloadModules();
	public:
		Core();
		// Runs the engine with the specified configuration file
		void Run(const std::filesystem::path& configPath);
		// Returns the entity registry
		EntityRegistry& GetEntityRegistry();
		// Requests a shutdown and begins the shutdown sequence
		void RequestShutdown();
		// Returns whether a module is loaded, given its name
		bool IsModuleLoaded(const std::string& moduleName);
		// Returns a module by name
		Module* GetModule(const std::string& moduleName);
		// Returns the singleton instance of the Core
		static Core* Get();
	};
}
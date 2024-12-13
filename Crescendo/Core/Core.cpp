#include "Core.hpp"
#include "Console.hpp"
#include "simdjson/simdjson.h"
#include "timestamp.hpp"

extern "C"
{
	__declspec(dllimport) void* __stdcall LoadLibraryA(const char* lpLibFileName);
	__declspec(dllimport) int __stdcall FreeLibrary(void* hLibModule);
	__declspec(dllimport) void* __stdcall GetProcAddress(void* hModule, const char* lpProcName);
}

namespace CrescendoEngine
{
	std::vector<std::string> ParseDependencies(const char* dependencies)
	{
		std::vector<std::string> result;
		if (dependencies && *dependencies)
		{
			std::istringstream stream(dependencies);
			std::string dependency;
			while (std::getline(stream, dependency, ','))
				result.push_back(dependency);
		}
		return result;
	}
	Core* Core::s_instance = nullptr;
	std::string Core::LoadConfig(const std::filesystem::path& path)
	{
		// Check if the file exists
		if (!std::filesystem::exists(path))
			Console::Fatal<std::runtime_error>("Config file does not exist: ", path);

		// Load the config file
		simdjson::dom::parser parser;
		simdjson::dom::element doc;
		auto error = parser.load(path.string()).get(doc);
		if (error)
			Console::Fatal<std::runtime_error>("Failed to parse config file: ", error);

		// Check for entrypoint
		if (!doc["entrypoint"].is_string())
			Console::Fatal<std::runtime_error>("Config file is missing entrypoint field");

		return std::string(doc["entrypoint"].get_string().value());
	}
	void Core::LoadModule(
		const std::filesystem::path& path, std::vector<ModuleData>& modules,
		std::unordered_set<std::string>& loadingModules, std::unordered_set<std::string>& loadedModules
	) {
		std::string moduleName = path.filename().string();

		// Check for cycles
		if (loadingModules.count(moduleName))
		{
			std::ostringstream cycleDetails;
			cycleDetails << "Cycle detected while loading module: '" << moduleName << "'\n\t" << "Current dependency chain: ";

			for (const auto& loadingModule : loadingModules)
				cycleDetails << loadingModule << " -> ";
			cycleDetails << moduleName;

			cycleDetails << "\n\tHint: Ensure modules do not have circular dependencies in their load order.";
			Console::Fatal<std::runtime_error>(cycleDetails.str());
		}

		// Skiped already loaded modules
		if (loadedModules.count(moduleName))
			return;

		// Mark module as currently loading
		loadingModules.insert(moduleName);

		// Load the DLL
		void* dllHandle = LoadLibraryA(path.string().c_str());
		if (dllHandle == nullptr)
			Console::Fatal<std::runtime_error>("Could not load module ", path);

		// Get the metadata
		auto GetMetadata = (GetMetadataFunc)GetProcAddress(dllHandle, "GetMetadata");
		if (GetMetadata == nullptr)
		{
			FreeLibrary(dllHandle);
			Console::Fatal<std::runtime_error>("Could not find metadata function \"GetMetadata\" in module ", path);
		}

		// Get the factory function
		CreateModuleFunc CreateModule = (CreateModuleFunc)GetProcAddress(dllHandle, "CreateModule");
		if (CreateModule == nullptr)
		{
			FreeLibrary(dllHandle);
			Console::Fatal<std::runtime_error>("Could not find factory function \"CreateModule\" in module ", path);
		}

		// Extract metadata data
		ModuleMetadata metadata = GetMetadata();
		Console::Log("Loaded module: ", metadata.name, " v", metadata.version, " by ", metadata.author, " (", metadata.description, ") - dependencies: ", metadata.dependencies);
		std::vector<std::string> dependencies = ParseDependencies(metadata.dependencies);

		// Load dependencies
		for (const auto& dependencyName : dependencies)
			LoadModule(dependencyName, modules, loadingModules, loadedModules);

		modules.push_back({ dllHandle, CreateModule, GetMetadata, nullptr });
		loadingModules.erase(moduleName);
		loadedModules.insert(moduleName);
	}
	void Core::InitializeModules(const std::vector<ModuleData>& modules)
	{
		for (const auto& module : modules)
		{
			Module* instance = module.createModule();
			if (instance == nullptr)
			{
				FreeLibrary(module.dllHandle);
				Console::Fatal<std::runtime_error>("Failed to create module instance");
			}
			m_loadedModules[module.getMetadata().name] = { module.dllHandle, module.createModule, module.getMetadata, std::unique_ptr<Module>(instance) };
			instance->OnLoad();
		}
	}
	void Core::MainLoop()
	{
		bool running = true;

		std::thread inputThread([&running] {
			std::string input;
			while (running)
			{
				std::getline(std::cin, input);

				if (input == "exit")
					running = false;
			}
		});

		
		Timestamp timestamp;
		double previous = timestamp.elapsed();
		double accumulator = 0.0;

		while (running)
		{
			const double elapsed = timestamp.elapsed();

			double frameTime = elapsed - previous;
			accumulator += frameTime;
			previous = elapsed;

			while (accumulator >= 0.5)
			{
				for (auto& [moduleName, module] : m_loadedModules)
				{
					module.module->OnUpdate(0.5);
				}
				accumulator -= 0.5;
			}
		}

		inputThread.join();
	}
	void Core::UnloadModules()
	{
		for (auto& [moduleName, module] : m_loadedModules)
			module.module->OnUnload();
		for (auto& [moduleName, module] : m_loadedModules)
		{
			if (module.module)
				module.module.reset();
			if (module.dllHandle)
				FreeLibrary(module.dllHandle);
		}
		m_loadedModules.clear();
	}
	Core::Core()
	{
		if (s_instance)
			Console::Fatal<std::runtime_error>("Core instance already exists");
		s_instance = this;
	}
	void Core::Run(const std::filesystem::path& configPath)
	{
		Console::Begin();
		Console::Log("Using config: ", configPath);

		std::string entrypoint = LoadConfig(configPath);

		std::vector<ModuleData> modules;
		// Used for tracking circular dependencies
		std::unordered_set<std::string> loadingModules;
		std::unordered_set<std::string> loadedModules;

		LoadModule(entrypoint, modules, loadingModules, loadedModules);
		InitializeModules(modules);
		MainLoop();
		UnloadModules();

		Console::Log("Core Shutdown, total time: ", static_cast<double>(Console::End<std::chrono::milliseconds>()) / 1000.0, "s");
	}
	EntityRegistry& Core::GetEntityRegistry()
	{
		return m_entityRegistry;
	}
	void Core::RequestShutdown()
	{
		Console::Log("Shutting down (does nothing)");
	}
	bool Core::IsModuleLoaded(const std::string& moduleName)
	{
		return m_loadedModules.find(moduleName) != m_loadedModules.end();
	}
	Module* Core::GetModule(const std::string& moduleName)
	{
		auto it = m_loadedModules.find(moduleName);
		if (it == m_loadedModules.end())
			return nullptr;
		return it->second.module.get();
	}
	Core* Core::Get() { return s_instance; }
}
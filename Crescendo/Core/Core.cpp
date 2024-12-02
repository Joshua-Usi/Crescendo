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
	void Core::LoadModule(const std::filesystem::path& path)
	{
		// Load the DLL
		void* dllHandle = LoadLibraryA(path.string().c_str());
		if (dllHandle == nullptr)
			Console::Fatal<std::runtime_error>("Could not load module ", path);

		// Get the metadata
		using GetMetadataFunc = ModuleMetadata(*)();
		auto GetMetadata = (GetMetadataFunc)GetProcAddress(dllHandle, "GetMetadata");
		if (GetMetadata == nullptr)
		{
			FreeLibrary(dllHandle);
			Console::Fatal<std::runtime_error>("Could not find metadata function \"GetMetadata\" in module ", path);
		}

		// Extract metadata data
		ModuleMetadata metadata = GetMetadata();
		Console::Log("Loaded module: ", metadata.name, " v", metadata.version, " by ", metadata.author, " (", metadata.description, ")");
		std::vector<std::string> dependencies = ParseDependencies(metadata.dependencies);
		if (dependencies.size() > 0)
		{
			for (const auto& dependency : dependencies)
				Console::Log("    ", metadata.name, " depends on ", dependency);
		} else
			Console::Log("   ", metadata.name, " has no dependencies");

		// Get the factory function
		using CreateModuleFunc = Module*(*)();
		CreateModuleFunc CreateModule = (CreateModuleFunc)GetProcAddress(dllHandle, "CreateModule");
		if (CreateModule == nullptr)
		{
			FreeLibrary(dllHandle);
			Console::Fatal<std::runtime_error>("Could not find factory function \"CreateModule\" in module ", path);
		}

		// Create the module instance
		Module* module = CreateModule();
		if (module == nullptr)
		{
			FreeLibrary(dllHandle);
			Console::Fatal<std::runtime_error>("Failed to create module instance from module ", path);
		}
		m_loadedModules.push_back({ dllHandle, std::unique_ptr<Module>(module) });
	}
	void Core::LoadModules(const std::vector<std::string>& modules)
	{
		for (const auto& module : modules)
			LoadModule(module);
		for (auto& module : m_loadedModules)
			module.module->OnLoad();
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
				for (auto& module : m_loadedModules)
					module.module->OnUpdate(0.5);
				Console::Log("Update...");
				accumulator -= 0.5;
			}
		}

		inputThread.join();
	}
	void Core::UnloadModules()
	{
		for (auto& module : m_loadedModules)
			module.module->OnUnload();
		for (auto& moduleData : m_loadedModules)
		{
			if (moduleData.module)
				moduleData.module.reset();
			if (moduleData.dllHandle)
				FreeLibrary(moduleData.dllHandle);
		}
		m_loadedModules.clear();
	}
	void Core::Run(const std::filesystem::path& configPath)
	{
		Console::Begin();
		Console::Log("Using config: ", configPath);

		std::string entrypoint = LoadConfig(configPath);

		std::vector<std::string> modules = { entrypoint };
		std::vector<

		LoadModule(entrypoint);
		MainLoop();
		UnloadModules();

		Console::Log("Core Shutdown, total time: ", static_cast<double>(Console::End<std::chrono::milliseconds>()) / 1000.0, "s");
	};
}
-- build and build intermediate output directory
output_dir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

universal_library_dirs = {
	"%{wks.location}/Crescendo/thirdparty/glfw/libs"
}

-- Universal settings
universal_includes = {
	"%{wks.name}/%{prj.name}/**.hpp",
	"%{wks.name}/%{prj.name}/**.cpp",
	"%{wks.name}/%{prj.name}/**.h",
	"%{wks.name}/%{prj.name}/**.c",
}
universal_include_dirs = {
	"%{wks.location}/Crescendo/Core",
	"%{wks.location}/Crescendo/thirdparty",
}
universal_debug_flags = {
	"MultiProcessorCompile",
	"NoMinimalRebuild",
}
universal_optimised_flags = {
	"LinkTimeOptimization",
	"MultiProcessorCompile",
	"NoMinimalRebuild",
}
universal_defines = {
	-- Remove architecture specific implementations. SSE2 is more than enough
	"SIMDJSON_IMPLEMENTATION_ICELAKE=0",
	"SIMDJSON_IMPLEMENTATION_HASWELL=0",
	"SIMDJSON_IMPLEMENTATION_ARM64=0",
	"SIMDJSON_IMPLEMENTATION_PPC64=0",
	"SIMDJSON_IMPLEMENTATION_LSX=0",
	"SIMDJSON_IMPLEMENTATION_LASX=0",
	"SIMDJSON_IMPLEMENTATION_WESTMERE=0"
}

module_prebuild_commands = {
	"python \"%{wks.location}GenerateEntrypoints.py\""
}

function applyDebugSettings()
	symbols "on"
	flags(universal_debug_flags)
end

function applyReleaseSettings()
	symbols "on"
	floatingpoint "fast"
	optimize "speed"
	flags(universal_optimised_flags)
end

function applyProductionSettings()
	symbols "off"
	floatingpoint "fast"
	optimize "speed"
	flags(universal_optimised_flags)
end

function applyCppSettings()
	language "C++"
	cppdialect "C++20"
	staticruntime "off"
end

function applyBuildsettings()
	targetdir ("bin/" .. output_dir)
	objdir ("bin-intermediate/" .. output_dir .. "/%{prj.name}")
	files(universal_includes)
	libdirs(universal_library_dirs)
	includedirs(universal_include_dirs)
	defines(universal_defines)
end

function applyBuildConfigSettings()
	filter "system:windows"
		systemversion "latest"
	filter "configurations:Debug"
		applyDebugSettings()
	filter "configurations:Release"
		applyReleaseSettings()
	filter "configurations:Production"
		applyProductionSettings()
end

function applyModuleSettings()
	kind "SharedLib"
	applyCppSettings()
	applyBuildsettings()
	prebuildcommands(module_prebuild_commands)
	defines("CS_BUILDING_MODULE_DLL")
	files { "%{prj.location}/Entrypoint.cpp" }
	links { "Core" }
	applyBuildConfigSettings();
end

function defineModule(moduleName, linked)
	project(moduleName)
		location("./%{wks.name}/" .. moduleName)

		if linked and #linked > 0 then
			links(linked)
		end

		applyModuleSettings()
end

workspace "Crescendo"
	architecture "x64"
	startproject "entrypoint"
	configurations {
		-- Intended for debugging
		"Debug",
		-- Intended for normal use
		"Release",
		-- Same as release but doesn't output debug symbols
		"Production"
	}

project "entrypoint"
	location "./%{wks.name}/entrypoint"
	kind "ConsoleApp"
	targetname "Crescendo"
	links { "Core" }
	dependson { "Main" }
	applyCppSettings()
	applyBuildsettings()
	applyBuildConfigSettings();
	filter "system:windows"
		files { '%{wks.location}/%{wks.name}/resources/resources.rc', '%{wks.location}/%{wks.name}/resources/**.ico' }

project "Core"
	location "./%{wks.name}/Core"
	kind "SharedLib"
	applyCppSettings()
	applyBuildsettings()
	defines("CS_BUILDING_CORE_DLL")
	links { "kernel32.lib" }
	files { "./%{wks.name}/thirdparty/simdjson/simdjson.cpp" }
	applyBuildConfigSettings();

-- Third party files
project "thirdparty"
	location "./%{wks.name}/thirdparty"
	kind "Utility"
	applyBuildsettings()

-- Resource files
project "resources"
	location "./%{wks.name}/resources"
	kind "Utility"
	applyBuildsettings()
	filter "system:windows"
  		files { 'resources.rc', '**.ico' }

---------------------------------------------------------------- Modules ----------------------------------------------------------------

defineModule("Main")
defineModule("WindowManager", { "glfw3.lib" })
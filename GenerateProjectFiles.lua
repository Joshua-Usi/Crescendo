-- build and build intermediate output directory
output_dir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
library_dirs = {}

-- Universal settings
universal_includes = {
	"%{wks.name}/%{prj.name}/**.hpp",
	"%{wks.name}/%{prj.name}/**.cpp",
	"%{wks.name}/%{prj.name}/**.h",
	"%{wks.name}/%{prj.name}/**.c",
}
universal_include_dirs = {
	"%{wks.location}/Crescendo/common",
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
	includedirs(universal_include_dirs)
	defines(universal_defines)
end

function applyModuleSettings()
	kind "SharedLib"
	applyCppSettings()
	applyBuildsettings()
	prebuildcommands(module_prebuild_commands)
	defines("CS_BUILDING_DLL")
	files { "%{prj.location}/Entrypoint.cpp" }
	links { "common" }
	filter "system:windows"
		systemversion "latest"
	filter "configurations:Debug"
		applyDebugSettings()
	filter "configurations:Release"
		applyReleaseSettings()
end

workspace "Crescendo"
	architecture "x64"
	startproject "Core"
	configurations {
		-- Intended for debugging
		"Debug",
		-- Intended for normal use
		"Release",
		-- Intended for release 
		"Production"
	}

-- Common files
project "common"
	location "./%{wks.name}/common"
	kind "SharedLib"

	applyCppSettings()
	applyBuildsettings()

	defines("CS_BUILDING_COMMON_DLL")

	filter "system:windows"
		systemversion "latest"
	filter "configurations:Debug"
		applyDebugSettings()
	filter "configurations:Release"
		applyReleaseSettings()

project "thirdparty"
	location "./%{wks.name}/thirdparty"
	kind "None"
	files(universal_includes)
	includedirs(universal_include_dirs)

-- Resource files
project "resources"
	location "./%{wks.name}/resources"
	kind "None"
	filter "system:windows"
  		files { 'resources.rc', '**.ico' }

project "Core"
	location "./%{wks.name}/Core"
	kind "ConsoleApp"
	targetname "Crescendo"

	applyCppSettings()
	applyBuildsettings()

	links {
		"common",
		-- Modules
		"Main",
		-- windows specific
		"kernel32.lib",
	}

	files {
		"./%{wks.name}/thirdparty/simdjson/simdjson.cpp"
	}

	filter "system:windows"
		systemversion "latest"
		files { '%{wks.location}/%{wks.name}/resources/resources.rc', '%{wks.location}/%{wks.name}/resources/**.ico' }
	filter "configurations:Debug"
		applyDebugSettings()
	filter "configurations:Release"
		applyReleaseSettings()

project "Main"
	location "./%{wks.name}/Main"
	applyModuleSettings()

project "Sandbox"
	location "%{wks.location}/Sandbox"
	kind "StaticLib"

	applyCppSettings()
	applyBuildsettings()

	filter "system:windows"
		systemversion "latest"
	filter "configurations:Debug"
		applyDebugSettings()
	filter "configurations:Release"
		applyReleaseSettings()
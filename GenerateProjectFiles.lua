outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
include_files = {
	"%{prj.name}/**.h",
	"%{prj.name}/**.cpp",
	"%{prj.name}/**.lib",
	-- future proof
	"%{prj.name}/**.c",
	"%{prj.name}/**.hpp",
	-- resource files
	"%{prj.name}/**.aps",
	"%{prj.name}/**.rc",
}
exclude_files = {}
library_dirs = {
	"%{wks.location}/Crescendo/ThirdParty/Discord/libs",
	"%{wks.location}/Crescendo/ThirdParty/glfw/libs",
}
engine_dirs = {
	"%{wks.location}/Crescendo",
	"%{wks.location}/Crescendo/Editor",
	"%{wks.location}/Crescendo/Engine",
	"%{wks.location}/Crescendo/Rendering",
	"%{wks.location}/Crescendo/Tools",
	"%{wks.location}/Crescendo/Utils",
	"%{wks.location}/Crescendo/ThirdParty",
	"%{wks.location}/Crescendo/ThirdParty/ImGui",
	"%{wks.location}/Crescendo/ThirdParty/ImGui/backends",
}
flags_debug = {
	"MultiProcessorCompile",
	"NoMinimalRebuild",
}
flags_optimised = {
	"LinkTimeOptimization",
	"MultiProcessorCompile",
	"NoMinimalRebuild",
}
sandbox_project_name = "Sandbox"

workspace "Crescendo"
	architecture "x64"
	startproject(sandbox_project_name)
	configurations {
		"Debug",
		-- Intended for development
		"Release",
		-- Intended for distribution and release
		"Prod",
	}

project "Crescendo"
	location "Crescendo"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"
	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-intermediate/" .. outputdir .. "/%{prj.name}")
	files(include_files)
	removefiles(exclude_files)
	includedirs(engine_dirs)
	libdirs(library_dirs)
	links {}
	filter "system:windows"
		systemversion "latest"
		defines {
			"CS_PLATFORM_WINDOWS",
			"CS_BUILD_DLL",
			"_CRT_SECURE_NO_WARNINGS",
			"GLFW_INCLUDE_NONE",
		}
	filter "configurations:Debug"
		defines "CS_DEBUG"
		symbols "on"
		flags(flags_debug)
	filter "configurations:Release"
		defines "CS_RELEASE"
		optimize "on"
		floatingpoint "Fast"
		optimize "speed"
		flags(flags_optimised)
	filter "configurations:Prod"
		defines "CS_PROD"
		optimize "on"
		floatingpoint "Fast"
		optimize "speed"
		flags(flags_optimised)

project(sandbox_project_name)
	location(sandbox_project_name)
	language "C++"
	cppdialect "C++20"
	staticruntime "on"
	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-intermediate/" .. outputdir .. "/%{prj.name}")
	files(include_files)
	includedirs(engine_dirs)
	libdirs(library_dirs)
	links {
		"Crescendo",
		--
		"glfw3_mt.lib",
		--
		"opengl32.lib",
		"user32.lib",
		"gdi32.lib",
		"shell32.lib",
		"kernel32.lib"
		-- "discord_game_sdk.dll.lib",
	}
	filter "system:windows"
		cppdialect "C++20"
		systemversion "latest"
		defines {
			"CS_PLATFORM_WINDOWS",
		}
	filter "configurations:Debug"
		defines "CS_DEBUG"
		symbols "on"
		flags(flags_debug)
		kind "ConsoleApp"
	filter "configurations:Release"
		defines "CS_RELEASE"
		optimize "on"
		floatingpoint "Fast"
		optimize "speed"
		flags(flags_optimised)
		kind "ConsoleApp"
	filter "configurations:Prod"
		defines "CS_PROD"
		optimize "on"
		floatingpoint "Fast"
		optimize "speed"
		flags(flags_optimised)
		kind "WindowedApp"
project("Tests")
	location("Tests")
	language "C++"
	cppdialect "C++20"
	staticruntime "on"
	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-intermediate/" .. outputdir .. "/%{prj.name}")
	files(include_files)
	includedirs(engine_dirs)
	libdirs(library_dirs)
	links {
		"Crescendo",
		--
		"glfw3_mt.lib",
		--
		"opengl32.lib",
		"user32.lib",
		"gdi32.lib",
		"shell32.lib",
		"kernel32.lib"
		-- "discord_game_sdk.dll.lib",
	}
	filter "system:windows"
		cppdialect "C++20"
		systemversion "latest"
		defines {
			"CS_PLATFORM_WINDOWS",
		}
	filter "configurations:Debug"
		defines "CS_DEBUG"
		symbols "on"
		flags(flags_debug)
		kind "ConsoleApp"
	filter "configurations:Release"
		defines "CS_RELEASE"
		optimize "on"
		floatingpoint "Fast"
		optimize "speed"
		flags(flags_optimised)
		kind "ConsoleApp"
	filter "configurations:Prod"
		defines "CS_PROD"
		optimize "on"
		floatingpoint "Fast"
		optimize "speed"
		flags(flags_optimised)
		kind "WindowedApp"
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
include_files = {
	"%{prj.name}/**.hpp",
	"%{prj.name}/**.cpp",
	-- C backward compatibility
	"%{prj.name}/**.h",
	"%{prj.name}/**.c",
	-- Volk
	"C:/VulkanSDK/1.3.250.1/Include/Volk/volk.c",
}
exclude_files = {}
library_dirs = {
	"%{wks.location}/Crescendo/Libraries/ThirdParty/glfw/libs",
}
engine_dirs = {
	"C:/VulkanSDK/1.3.250.1/Include",
	"%{wks.location}/Crescendo",
	"%{wks.location}/Crescendo/Libraries/ThirdParty",
	"%{wks.location}/Crescendo/Libraries/ThirdParty/ImGui",
	"%{wks.location}/Crescendo/Libraries/ThirdParty/ImGui/backends",
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
			"_CRT_SECURE_NO_WARNINGS",
			"GLFW_INCLUDE_NONE",
			"VK_NO_PROTOTYPES",
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
		"user32.lib",
		"gdi32.lib",
		"shell32.lib",
		"kernel32.lib"
	}
	filter "system:windows"
		cppdialect "C++20"
		systemversion "latest"
		defines {
			"CS_PLATFORM_WINDOWS",
			"VK_NO_PROTOTYPES",
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
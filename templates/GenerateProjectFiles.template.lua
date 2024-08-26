vulkan_ver = "<vulkan_ver>"
project_name = "<project_name>"
output_dir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
include_files = {
	"%{prj.name}/**.hpp",
	"%{prj.name}/**.cpp",
	-- C backward compatibility
	"%{prj.name}/**.h",
	"%{prj.name}/**.c",
	-- Volk
	"C:/VulkanSDK/" .. vulkan_ver .. "/Include/Volk/volk.c",
}
exclude_files = {}
library_dirs = {
	"%{wks.location}/Crescendo/Libraries/ThirdParty/glfw/libs",
}
engine_dirs = {
	"C:/VulkanSDK/" .. vulkan_ver .. "/Include",
	"%{wks.location}/Crescendo",
	"%{wks.location}/Crescendo/Libraries/ThirdParty",
	"%{wks.location}/Crescendo/Libraries/ThirdParty/ImGui",
	"%{wks.location}/Crescendo/Libraries/ThirdParty/ImGui/backends",
	"%{wks.location}/Crescendo/cs_std",
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
universal_defines = {
	"CS_PLATFORM_WINDOWS",
	"_CRT_SECURE_NO_WARNINGS",
	"GLFW_INCLUDE_NONE",
	"VK_NO_PROTOTYPES",
}

workspace(project_name)
	architecture "x64"
	startproject(project_name)
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
	targetdir ("bin/" .. output_dir .. "/%{prj.name}")
	objdir ("bin-intermediate/" .. output_dir .. "/%{prj.name}")
	files(include_files)
	removefiles(exclude_files)
	includedirs(engine_dirs)
	libdirs(library_dirs)
	links {}
	filter "system:windows"
		systemversion "latest"
		defines(universal_defines)
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

project(project_name)
	location(project_name)
	language "C++"
	cppdialect "C++20"
	staticruntime "on"
	targetdir ("bin/" .. output_dir .. "/%{prj.name}")
	objdir ("bin-intermediate/" .. output_dir .. "/%{prj.name}")
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
		defines(universal_defines)
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
#pragma once

#include "Core/common.hpp"

namespace Crescendo::Engine
{
	/// <summary>
	/// Defines window properties
	/// </summary>
	struct WindowProperties
	{
		std::string title;
		uint32_t width, height;

		WindowProperties(
			const std::string& windowTitle = "Crescendo",
			uint32_t windowWidth = 1280,
			uint32_t windowHeight = 720
		) : title(windowTitle), width(windowWidth), height(windowHeight) {};
	};

	/// <summary>
	/// Window interface for desktop systems
	/// </summary>
	class Window
	{
	protected:
		struct WindowData
		{
			std::string title = "";
			Window* windowPointer = NULL;
			uint32_t width = 0, height = 0;
			bool vSync = false, isCursorLocked = false, isOpen = false;
		} data;
	public:
		/// <summary>
		/// Global method for creating a window on specific platforms
		/// </summary>
		static shared<Window> Create(const WindowProperties& props = WindowProperties());
		virtual ~Window() {};
		/// <summary>
		/// Manually close the window
		/// </summary>
		virtual void Close() = 0;
		/// <summary>
		/// Run once at the beginning of each frame
		/// </summary>
		virtual void OnUpdate() = 0;
		/// <summary>
		/// Run once at the end of each frame
		/// </summary>
		virtual void OnLateUpdate() = 0;

		/// <summary>
		/// Get the width of the window
		/// </summary>
		/// <returns>Width of the window as an integer</returns>
		virtual uint32_t GetWidth() const = 0;
		/// <summary>
		/// Get the height of the window
		/// </summary>
		/// <returns>Height of the window as an integer</returns>
		virtual uint32_t GetHeight() const = 0;
		/// <summary>
		/// Get the aspect ratio of the window
		/// </summary>
		/// <returns>Aspect ratio of the float</returns>
		virtual float GetAspectRatio() const = 0;
		/// <summary>
		/// Returns a void pointer to the native window pointer
		/// </summary>
		/// <returns>Pointer to native window</returns>
		virtual void* GetNative() const = 0;
		/// <summary>
		/// Returns the refresh rate of the given window in Hz
		/// </summary>
		/// <returns>Refresh rate in Hz</returns>
		virtual int32_t GetRefreshRate() const = 0;
		/// <summary>
		/// Returns the name of the window
		/// </summary>
		/// <returns>Window name</returns>
		virtual std::string GetName() const = 0;

		/// <summary>
		/// Interface for enabling / disabling vsync
		/// </summary>
		/// <param name="isEnabled"></param>
		virtual void SetVSync(bool isEnabled) = 0;
		/// <summary>
		/// Interface for enabling / disabling cursor locks
		/// </summary>
		/// <param name="isEnabled"></param>
		virtual void SetCursorLock(bool isEnabled) = 0;
		/// <summary>
		/// Set the name of the window
		/// </summary>
		/// <param name="name">New name of the window</param>
		virtual void SetName(const std::string& name) = 0;

		/// <summary>
		/// Determines if a window is vsynced
		/// </summary>
		/// <returns>Boolean</returns>
		virtual bool IsVSynced() const = 0;
		/// <summary>
		/// Determines if the cursor is locked on the screen
		/// </summary>
		/// <returns>Boolean</returns>
		virtual bool IsCursorLocked() const = 0;
		/// <summary>
		/// Determines if a window is open
		/// </summary>
		virtual bool IsOpen() const = 0;
	};
}
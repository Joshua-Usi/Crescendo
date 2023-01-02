#pragma once

#include "core/core.h"

namespace Crescendo::Engine {
	struct WindowProps {
		std::string title;
		cs::int32 width;
		cs::int32 height;

		WindowProps(const std::string& windowTitle = "Crescendo", cs::int32 windowWidth = 1280, cs::int32 windowHeight = 720) {
			title = windowTitle;
			width = windowWidth;
			height = windowHeight;
		}
	};
	/// <summary>
	/// Window interface for desktop systems
	/// </summary>
	class CS_API Window {
	public:
		virtual ~Window() {};
		/// <summary>
		/// Run once every frame
		/// </summary>
		virtual void OnUpdate() = 0;

		/// <summary>
		/// Get the width of the window
		/// </summary>
		/// <returns>Width of the window as an integer</returns>
		virtual cs::int32 GetWidth() const = 0;
		/// <summary>
		/// Get the height of the window
		/// </summary>
		/// <returns>Height of the window as an integer</returns>
		virtual cs::int32 GetHeight() const = 0;
		/// <summary>
		/// Returns a void pointer to the native window pointer
		/// </summary>
		/// <returns>Pointer to native window</returns>
		virtual void* GetNative() const = 0;
		/// <summary>
		/// Returns the refresh rate of the given window in Hz
		/// </summary>
		/// <returns>Refresh rate in Hz</returns>
		virtual cs::int32 GetRefreshRate() const = 0;

		/// <summary>
		/// Interface for enabling vsync
		/// </summary>
		/// <param name="isEnabled"></param>
		virtual void SetVSync(bool isEnabled) = 0;

		/// <summary>
		/// Determines if a window is vsynced
		/// </summary>
		virtual bool IsVSynced() const = 0;
		/// <summary>
		/// Determines if a window is open
		/// </summary>
		virtual bool IsOpen() const = 0;

		/// <summary>
		/// Global method for creating a window on specific platforms
		/// </summary>
		static Window* Create(const WindowProps& props = WindowProps());
	};
}
#pragma once

#include "core/core.h"

#include "interfaces/RendererAPI.h"

namespace Crescendo::Engine {
	struct WindowProps {
		Rendering::RendererAPI::API graphicsAPI;
		int32_t width;
		int32_t height;
		const char* title;

		WindowProps(const char* windowTitle = "Crescendo", int32_t windowWidth = 1280, int32_t windowHeight = 720, Rendering::RendererAPI::API windowGraphicsAPI = Rendering::RendererAPI::API::OpenGL) {
			this->graphicsAPI = windowGraphicsAPI;
			this->title = windowTitle;
			this->width = windowWidth;
			this->height = windowHeight;
		}
	};
	/// <summary>
	/// Window interface for desktop systems
	/// </summary>
	class Window {
	public:
		virtual ~Window() {};
		/// <summary>
		/// Manually close the window
		/// </summary>
		virtual void Close() = 0;
		/// <summary>
		/// Run once every frame
		/// </summary>
		virtual void OnUpdate() = 0;
		/// <summary>
		/// Run once at the end of every frame
		/// </summary>
		virtual void OnLateUpdate() = 0;

		/// <summary>
		/// Get the width of the window
		/// </summary>
		/// <returns>Width of the window as an integer</returns>
		virtual int32_t GetWidth() const = 0;
		/// <summary>
		/// Get the height of the window
		/// </summary>
		/// <returns>Height of the window as an integer</returns>
		virtual int32_t GetHeight() const = 0;
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

		/// <summary>
		/// Global method for creating a window on specific platforms
		/// </summary>
		static Window* Create(const WindowProps& props = WindowProps());
	};
}
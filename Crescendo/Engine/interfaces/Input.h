#pragma once

#include "core/core.h"
#include "scancodes/keyCodes.h"
#include "scancodes/mouseCodes.h"

namespace Crescendo::Engine {
	/// <summary>
	/// Only one instance of this class should ever exist even throughout multiple windows / surfaces
	/// </summary>
	class CS_API Input {
	public:
		/// <summary>
		/// Returns true once for a given key, then never return true again until the key is released and then pressed again
		/// Useful for one time key events
		/// </summary>
		inline static bool GetKeyDown(Key keyCode) {
			return self->KeyDownImpl(keyCode);
		}
		/// <summary>
		/// Returns true if a given key is down
		/// </summary>
		/// <param name="keyCode">Enumerated keycode</param>
		inline static bool GetKeyPressed(Key keyCode) {
			return self->KeyPressedImpl(keyCode);
		}

		/// <summary>
		/// Return the current mouse position x relative to the top-left of the window
		/// </summary>
		/// <returns>X position of mouse as a double</returns>
		inline static double GetMousePositionX() {
			return self->MousePositionXImpl();
		}
		/// <summary>
		/// Return the current mouse position y relative to the top-left of the window
		/// </summary>
		/// <returns>Y position of mouse as a double</returns>
		inline static double GetMousePositionY() {
			return self->MousePositionYImpl();
		}

		// TODO CRESCENDO maybe at some point add some vector2 functions for mouse position

		/// <summary>
		/// Returns true once for a given button, then never returns true again until the button is released and pressed again
		/// Useful for one time key events
		/// </summary>
		/// <param name="button">Enumerated mouse button</param>
		inline static bool GetMouseButtonDown(MouseButton button) {
			return self->MouseButtonDownImpl(button);
		}
		/// <summary>
		/// Returns true for a given mouse key
		/// </summary>
		/// <param name="button">Enumerated mouse button</param>
		inline static bool GetMouseButtonPressed(MouseButton button) {
			return self->MouseButtonPressedImpl(button);
		}
	protected:
		// implementations for platform specifics
		virtual bool KeyDownImpl(Key keyCode) = 0;
		virtual bool KeyPressedImpl(Key keyCode) = 0;

		virtual double MousePositionXImpl() = 0;
		virtual double MousePositionYImpl() = 0;

		virtual bool MouseButtonDownImpl(MouseButton button) = 0;
		virtual bool MouseButtonPressedImpl(MouseButton button) = 0;
	private:
		static Input* self;
	};
}
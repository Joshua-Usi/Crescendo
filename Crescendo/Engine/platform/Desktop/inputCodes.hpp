#pragma once

#include <cstdint>

/* * * * * * * * * * * * * * * * * * * * * * * * * * *
 *	The C++ standard makes gaurantees on enums		 *
 *	That declaring an enum without specifying values *
 *	Will result in the first value being 0, and each *
 *	Subsequent value being one greater than the last.*
 * * * * * * * * * * * * * * * * * * * * * * * * * * */
namespace Crescendo::Engine
{
	enum class MouseButton : uint32_t
	{
		// Specific commonly used button
		Left = 0, Middle, Right,

		// Non-specific unknown bound buttons
		One, Two, Three, Four, Five, Six, Seven, Eight,

		// Not intended to be used as a bound button
		BUTTON_COUNT
	};
	enum class Key : uint32_t
	{
		// Symbols
		Space = 0, Apostrophe, Comma, Minus, Period, Slash,

		// Numbers
		Zero, One, Two, Three, Four, Five, Six, Seven, Eight, Nine,

		// Symbols
		Semicolon, Equal,

		// Letters
		A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

		BracketLeft, Backslash, BracketRight, Grave,

		// Presumably this is for apple keyboards?
		World1, World2,

		Escape, Enter, Tab, Backspace, Insert, Delete,

		ArrowRight, ArrowLeft, ArrowDown, ArrowUp,

		PageUp, PageDown,
		Home, End,
		CapsLock, ScrollLock,
		NumLock, PrtSc, Pause,

		F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12, F13, F14, F15, F16, F17, F18, F19, F20, F21, F22, F23, F24, F25,

		Keypad0, Keypad1, Keypad2, Keypad3, Keypad4, Keypad5, Keypad6, Keypad7, Keypad8, Keypad9,
		KeypadDecimal, KeypadDivide, KeypadMultiply, KeypadSubtract, KeypadAdd, KeypadEnter, KeypadEqual,

		ShiftLeft, ControlLeft, AltLeft, SuperLeft,
		ShiftRight, ControlRight, AltRight, SuperRight,

		// Support for specific keys per platform
		WindowsLeft, MacCommandLeft,
		WindowsRight, MacCommandRight,

		Menu,

		// Not intended to be used as a bound key
		KEY_COUNT
	};
}
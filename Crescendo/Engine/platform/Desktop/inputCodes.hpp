#pragma once

#include "common.hpp"

CS_NAMESPACE_BEGIN
{
	enum class MouseButton : uint32_t
	{
		Left = 0, Middle, Right,
		One, Two, Three, Four, Five, Six, Seven, Eight,
		BUTTON_COUNT
	};
	enum class Key : uint32_t
	{
		Space = 0, Apostrophe, Comma, Minus, Period, Slash,
		Zero, One, Two, Three, Four, Five, Six, Seven, Eight, Nine,
		Semicolon, Equal,
		A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
		BracketLeft, Backslash, BracketRight, Grave,
		World1, World2, // Presumably this is for apple keyboards?
		Escape, Enter, Tab, Backspace, Insert, Delete,
		ArrowRight, ArrowLeft, ArrowDown, ArrowUp,
		PageUp, PageDown, Home, End, CapsLock, ScrollLock, NumLock, PrtSc, Pause,
		F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12, F13, F14, F15, F16, F17, F18, F19, F20, F21, F22, F23, F24, F25,
		Keypad0, Keypad1, Keypad2, Keypad3, Keypad4, Keypad5, Keypad6, Keypad7, Keypad8, Keypad9,
		KeypadDecimal, KeypadDivide, KeypadMultiply, KeypadSubtract, KeypadAdd, KeypadEnter, KeypadEqual,
		ShiftLeft, ControlLeft, AltLeft, SuperLeft,
		ShiftRight, ControlRight, AltRight, SuperRight,
		WindowsLeft, MacCommandLeft,
		WindowsRight, MacCommandRight,
		Menu,
		KEY_COUNT
	};
}
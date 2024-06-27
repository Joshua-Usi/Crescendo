#pragma once

#include "Engine/platform/Desktop/InputCodes.hpp"

CS_NAMESPACE_BEGIN
{
	class Window;

	class Input
	{
	public:
		struct Specification
		{
			Window* window;
			Specification() : window(nullptr) {}
			Specification(Window* window) : window(window) {}
		};
	public:
		static std::unique_ptr<Input> Create(const Specification& spec = Specification());
		virtual ~Input() = default;
		virtual bool GetKeyDown(Key keyCode) = 0;
		virtual bool GetKeyPressed(Key keyCode) = 0;
		virtual double GetMouseX() = 0;
		virtual double GetMouseY() = 0;
		virtual bool GetMouseDown(MouseButton button) = 0;
		virtual bool GetMousePressed(MouseButton button) = 0;
		virtual void PollEvents() = 0;
	};
}
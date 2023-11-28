#pragma once

#include "Engine/platform/Desktop/InputCodes.hpp"

CS_NAMESPACE_BEGIN
{
	class Input
	{
	private:
		static Input* self;
	public:
		static bool GetKeyDown(Key keyCode)
		{
			return self->KeyDownImpl(keyCode);
		}
		static bool GetKeyPressed(Key keyCode)
		{
			return self->KeyPressedImpl(keyCode);
		}
		static double GetMouseX()
		{
			return self->MousePositionXImpl();
		}
		static double GetMouseY()
		{
			return self->MousePositionYImpl();
		}
		static bool GetMouseButtonDown(MouseButton button)
		{
			return self->MouseButtonDownImpl(button);
		}
		static bool GetMouseButtonPressed(MouseButton button)
		{
			return self->MouseButtonPressedImpl(button);
		}
		static void PollEvents()
		{
			self->PollEventsImpl();
		}
	protected:
		virtual bool KeyDownImpl(Key keyCode) = 0;
		virtual bool KeyPressedImpl(Key keyCode) = 0;
		virtual double MousePositionXImpl() = 0;
		virtual double MousePositionYImpl() = 0;
		virtual bool MouseButtonDownImpl(MouseButton button) = 0;
		virtual bool MouseButtonPressedImpl(MouseButton button) = 0;
		virtual void PollEventsImpl() = 0;
	};
}
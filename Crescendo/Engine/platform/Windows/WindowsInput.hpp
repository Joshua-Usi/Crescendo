#pragma once

#include "Engine/interfaces/Input.hpp"

namespace Crescendo::Engine
{
	class WindowsInput : public Input
	{
	private:
		bool isKeyDown[static_cast<int>(Key::KEY_COUNT)] = { false };
		bool isMouseButtonDown[static_cast<int>(MouseButton::BUTTON_COUNT)] = { false };
	protected:
		// implementations for platform specific
		virtual bool KeyDownImpl(Key keyCode) override final;
		virtual bool KeyPressedImpl(Key keyCode) override final;

		virtual double MousePositionXImpl() override final;
		virtual double MousePositionYImpl() override final;

		virtual bool MouseButtonDownImpl(MouseButton button) override final;
		virtual bool MouseButtonPressedImpl(MouseButton button) override final;

		virtual void PollEventsImpl() override final;
	};
}
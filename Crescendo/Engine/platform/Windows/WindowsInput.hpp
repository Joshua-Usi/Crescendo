#pragma once

#include "Engine/interfaces/Input.hpp"

#include <array>

namespace Crescendo::Engine
{
	class WindowsInput : public Input
	{
	private:
		double mouseX = 0, mouseY = 0, pMouseX = 0, pMouseY = 0;
	protected:
		// implementations for platform specific
		virtual bool KeyDownImpl(Key keyCode) const override final;
		virtual bool KeyPressedImpl(Key keyCode) const override final;

		virtual double MousePositionXImpl() const override final;
		virtual double MousePositionYImpl() const override final;

		virtual bool MouseButtonDownImpl(MouseButton button) const override final;
		virtual bool MouseButtonPressedImpl(MouseButton button) const override final;
	};
}
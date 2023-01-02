#pragma once

#include "core/core.h"
#include "interfaces/Input.h"

namespace Crescendo::Engine
{
	class WindowsInput : public Input
	{
	protected:
		// implementations for platform specific
		virtual bool KeyDownImpl(Key keyCode) override;
		virtual bool KeyPressedImpl(Key keyCode) override;

		virtual double MousePositionXImpl() override;
		virtual double MousePositionYImpl() override;

		virtual bool MouseButtonDownImpl(MouseButton button) override;
		virtual bool MouseButtonPressedImpl(MouseButton button) override;
	};
}
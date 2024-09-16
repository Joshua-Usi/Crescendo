#pragma once
#include "common.hpp"
#include "Component.hpp"

CS_NAMESPACE_BEGIN
{
	struct alignas(uint16_t) TextRenderer : Component
	{
		enum class RenderMode : uint8_t
		{
			// The text appears in the 3D world, this is digetic UI
			WorldSpace = 0,
			// The text appears directly on the screen, the position is in pixels
			ScreenSpace
		};
		enum class Position : uint8_t
		{
			// Defines the position of the text relative to the screen
			Absolute = 0, TopLeft, TopCenter, TopRight, MiddleLeft, MiddleCenter, MiddleRight, BottomLeft, BottomCenter, BottomRight,
		};
		RenderMode renderMode;
		Position position;
		
		TextRenderer(RenderMode renderMode, Position position = Position::Absolute)
			: renderMode(renderMode), position(position) {}
	};
}
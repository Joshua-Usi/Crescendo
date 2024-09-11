#pragma once

#include "common.hpp"
#include "Component.hpp"
#include "utils/Color.hpp"

CS_NAMESPACE_BEGIN
{
	struct Text : Component
	{
		// No justified
		enum class Alignment : uint8_t { Left = 0, Center = 1, Right = 2 };
		enum class Unit : uint8_t { Px = 0, Vw = 1, Vh = 2 };

		std::string text;
		std::string font;
		Color color;
		// if in worldspace, this is the size in world units and will not care about Unit type, if in screenspace, this is the size in pixels
		float fontSize;
		float lineSpacing; // 1.0f is normal, multiplier
		Alignment alignment;
		Unit unit;

		Text(const std::string& text, std::string font, Color color, float fontSize, float lineSpacing, Alignment alignment)
			: text(text), font(font), color(color), fontSize(fontSize), lineSpacing(lineSpacing), alignment(alignment), unit(Unit::Px)
		{}
	};
}
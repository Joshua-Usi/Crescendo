#pragma once
#include "common.hpp"

CS_NAMESPACE_BEGIN
{
	struct alignas(uint32_t) Color
	{
		uint8_t r, g, b, a;
		Color() : r(0), g(0), b(0), a(0) {}

		Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) : r(r), g(g), b(b), a(a) {}
		Color(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b), a(255) {}
		Color (uint8_t gray, uint8_t a) : r(gray), g(gray), b(gray), a(a) {}	
		Color (uint8_t gray) : r(gray), g(gray), b(gray), a(255) {}

		uint32_t GetPacked() const { return (a << 24) | (b << 16) | (g << 8) | r; }
		inline static Color FromHSV(float h, float s, float v)
		{
			float c = v * s;
			float x = c * (1 - std::fabsf(fmodf(h / 60.0f, 2) - 1));
			float m = v - c;
			float r, g, b;
			if (h < 60) { r = c; g = x; b = 0; }
			else if (h < 120) { r = x; g = c; b = 0; }
			else if (h < 180) { r = 0; g = c; b = x; }
			else if (h < 240) { r = 0; g = x; b = c; }
			else if (h < 300) { r = x; g = 0; b = c; }
			else { r = c; g = 0; b = x; }
			return Color(
				static_cast<uint8_t>((r + m) * 255.0f),
				static_cast<uint8_t>((g + m) * 255.0f),
				static_cast<uint8_t>((b + m) * 255.0f)
			);
		}
		// from packed uint32_t
		inline static Color FromPacked(uint32_t packed) { return Color((packed & 0xFF), (packed >> 8) & 0xFF, (packed >> 16) & 0xFF, (packed >> 24) & 0xFF); }
	};
	inline Color GenerateColorFromIndex(uint32_t index) {
		float h = ((index * 2654435761) % 360); // Hue between 0 and 1
		float s = 0.75f;                  // Set saturation to 75%
		float v = 0.85f;                  // Set value/brightness to 85%

		// Use the FromHSV function to generate the color
		return Color::FromHSV(h, s, v);
	}
}
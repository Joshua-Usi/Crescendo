#pragma once

#include <cstdint>
#include <vector>

namespace cs_std
{
	struct image
	{
		std::vector<uint8_t> data;
		uint16_t width, height, channels;

		image() : data(), width(0), height(0), channels(0) {}
		image(const std::vector<uint8_t>& data, uint16_t width, uint16_t height, uint16_t channels) : data(data), width(width), height(height), channels(channels) {}
		image(uint16_t width, uint16_t height, uint16_t channels) : data(width * height * channels), width(width), height(height), channels(channels) {}
		~image() = default;
		image(const image& other) : data(other.data), width(other.width), height(other.height), channels(other.channels) {}
		image& operator=(const image& other) { if (this != &other) { data = other.data; width = other.width; height = other.height; channels = other.channels; } return *this; }
		image(image&& other) noexcept : data(std::move(other.data)), width(other.width), height(other.height), channels(other.channels) {}
		image& operator=(image&& other) noexcept { if (this != &other) { data = std::move(other.data); width = other.width; height = other.height; channels = other.channels; } return *this; }
	};
}
#pragma once
#include "common.hpp"

CS_NAMESPACE_BEGIN
{
	// Stack based push constant buffer
	// Holds 128 bytes due to vulkan mandating a minimum of 128 bytes
	// Follows std430 layout rules
	template<size_t bufferSize = 128>
	class PushConstantBuffer
	{
	private:
		std::array<std::byte, bufferSize> data;
		size_t size;
		// Defines the seperator between the vertex and fragment push constant data
		size_t separator;
	public:
		PushConstantBuffer() : data(), size(0), separator(0)
		{
			static_assert(bufferSize % 4 == 0, "Push constant buffer size must be a multiple of 4");
		}
		template<typename T>
		PushConstantBuffer& Push(T value) {
			if (size + sizeof(T) > bufferSize)
				throw std::runtime_error("Push constant buffer overflow");
			// calculate alignment
			// vec3 align to 16 bytes
			size_t alignment = (sizeof(T) == 3 * sizeof(float)) ? 4 * sizeof(float) : sizeof(T);
			size_t prePadding = (alignment - (size % alignment)) % alignment;
			size_t postPadding = (sizeof(T) == 3 * sizeof(float)) ? sizeof(float) : 0;

			memcpy(data.data() + size + prePadding, &value, sizeof(T));
			size += prePadding + sizeof(T) + postPadding;
			return *this;
		}
		void* Get(size_t offset = 0) { return data.data() + offset; }
		size_t GetSize(size_t offset = 0) { return size - offset; }
		PushConstantBuffer& Separate()
		{
			separator = size;
			return *this;
		}
	};
}
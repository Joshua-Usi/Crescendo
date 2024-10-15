#pragma once
#include "common.hpp"

CS_NAMESPACE_BEGIN
{
	// Stack based push constant buffer
	// Holds 128 bytes due to vulkan mandating a minimum of 128 bytes
	class PushConstantBuffer
	{
	private:
		std::array<std::byte, 128> data;
		size_t size;
	public:
		PushConstantBuffer() : data(), size(0) {}
		template<typename T>
		PushConstantBuffer& Push(T value) {
			if (size + sizeof(T) > 128)
				throw std::runtime_error("Push constant buffer overflow");
			memcpy(data.data() + size, &value, sizeof(T));
			size += sizeof(T);
			return *this;
		}
		void* Get(size_t offset = 0) { return data.data() + offset; }
		size_t GetSize() { return size; }
	};
}
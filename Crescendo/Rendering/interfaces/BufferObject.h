#pragma once

#include "core/core.h"

#include "GraphicsAPI.h"

namespace Crescendo::Rendering
{
	enum class BufferType
	{
		ArrayBuffer,
		ElementArrayBuffer,
	};
	class BufferObject
	{
	public:
		virtual ~BufferObject() {}

		virtual void SetData(void* pointer, uint64_t size) = 0;
		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		static BufferObject* Create(BufferType bufferType);
	};
}
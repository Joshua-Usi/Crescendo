#pragma once

#include "core/core.h"
#include "GraphicsAPI.h"

namespace Crescendo::Rendering
{
	enum class BufferType
	{
		Vertex,
		Index,
	};
	class BufferObject
	{
	public:
		virtual ~BufferObject() {}

		/// <summary>
		/// Uploads data to the gpu. Automatically detects the type of the data
		/// </summary>
		/// <param name="pointer">pointer to the beginning of the data</param>
		/// <param name="count">The number of elements in the data</param>
		virtual void SetData(void* pointer, uint32_t count) = 0;
		/// <summary>
		/// Bind the buffer object for changes / use
		/// </summary>
		virtual void Bind() = 0;
		/// <summary>
		/// Unbind the buffer to prevent unintended changing
		/// </summary>
		virtual void Unbind() = 0;
		/// <summary>
		/// Return the number of elements in the data
		/// </summary>
		/// <returns></returns>
		virtual uint32_t GetCount() = 0;

		/// <summary>
		/// Creates a RenderAPI specific BufferObject
		/// </summary>
		/// <param name="bufferType">The type of buffer to create: Index or Vertex</param>
		/// <returns>A polymorphic BufferObject</returns>
		static BufferObject* Create(BufferType bufferType);
	};
}
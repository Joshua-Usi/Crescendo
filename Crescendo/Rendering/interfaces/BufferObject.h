#pragma once

#include "core/core.h"
#include "GraphicsAPI.h"

#include <vector>

namespace Crescendo::Rendering
{
	enum class ShaderDataType
	{
		None = 0,
		Bool = 1,
		Int = 4, Int2 = 4 * 2, Int3 = 4 * 3, Int4 = 4 * 4,
		Float = 4, Float2 = 4 * 2, Float3 = 4 * 3, Float4 = 4 * 4,
		Double = 8, Double2 = 8 * 2, Double3 = 8 * 3, Double4 = 8 * 4,

		Mat3 = 4 * 3 * 3, Mat4 = 4 * 4 * 4,
	};
	enum class BufferType
	{
		Vertex,
		Index,
	};
	struct BufferElement
	{
		const char* name;
		ShaderDataType type;
		uint32_t offset;
		uint32_t size;

		BufferElement(ShaderDataType type, const char* name)
		{
			this->name = name;
			this->type = type;
			this->size = (uint32_t)type;
			this->offset = 0;
		}
	};
	class BufferLayout
	{
	private:
		std::vector<BufferElement> elements;
		uint32_t stride = 0;
		void SetOffsetsAndStride()
		{
			uint32_t offset = 0;
			this->stride = 0;
			for (auto& element : this->elements)
			{
				element.offset = offset;
				offset += element.size;
				this->stride += element.size;
			}
		}
	public:
		BufferLayout(const std::initializer_list<BufferElement>& element) : elements(element)
		{
			this->SetOffsetsAndStride();
		}
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
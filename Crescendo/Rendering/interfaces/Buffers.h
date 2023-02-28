#pragma once

#include "core/core.h"

#include <vector>

namespace Crescendo::Rendering
{
	enum class ShaderDataType
	{
		None = 0,
		Bool,
		Int, Int2, Int3, Int4,
		Float, Float2, Float3, Float4,
		Double, Double2, Double3, Double4,
		Mat3, Mat4,
	};
	/// <summary>
	/// Returns the size of the shader type in bytes
	/// </summary>
	static uint32_t GetShaderDataTypeSize(ShaderDataType type)
	{
		switch (type)
		{
		case ShaderDataType::Bool: return 1;

		case ShaderDataType::Int: return 4;
		case ShaderDataType::Int2: return 4 * 2;
		case ShaderDataType::Int3: return 4 * 3;
		case ShaderDataType::Int4: return 4 * 4;

		case ShaderDataType::Float: return 4;
		case ShaderDataType::Float2: return 4 * 2;
		case ShaderDataType::Float3: return 4 * 3;
		case ShaderDataType::Float4: return 4 * 4;

		case ShaderDataType::Double: return 8;
		case ShaderDataType::Double2: return 8 * 2;
		case ShaderDataType::Double3: return 8 * 3;
		case ShaderDataType::Double4: return 8 * 4;

		case ShaderDataType::Mat3: return 4 * 3 * 3;
		case ShaderDataType::Mat4: return 4 * 4 * 4;
		}

		CS_ASSERT(false, "Unknown ShaderDataType");
		return 0;
	}
	struct BufferElement
	{
		const char* name;
		ShaderDataType type;
		uint32_t offset;
		uint32_t size;
		bool normalized;

		BufferElement() 
			: name(""), type(ShaderDataType::None), size(0), offset(0), normalized(false) {}
		BufferElement(ShaderDataType type, const char* name, bool normalized = false)
			: name(name), type(type), size(GetShaderDataTypeSize(type)), offset(0), normalized(normalized) {}

		uint32_t GetComponentCount() const
		{
			switch (type)
			{
			case ShaderDataType::Bool: return 1;

			case ShaderDataType::Int: return 1;
			case ShaderDataType::Int2: return 2;
			case ShaderDataType::Int3: return 3;
			case ShaderDataType::Int4: return 4;

			case ShaderDataType::Float: return 1;
			case ShaderDataType::Float2: return 2;
			case ShaderDataType::Float3: return 3;
			case ShaderDataType::Float4: return 4;

			case ShaderDataType::Double: return 1;
			case ShaderDataType::Double2: return 2;
			case ShaderDataType::Double3: return 3;
			case ShaderDataType::Double4: return 4;

			case ShaderDataType::Mat3: return 3 * 3;
			case ShaderDataType::Mat4: return 4 * 4;
			}

			CS_ASSERT(false, "Unknown ShaderDataType");
			return 0;
		}
	};
	class BufferLayout
	{
	private:
		std::vector<BufferElement> elements;
		uint32_t stride = 0;
		void SetOffsetsAndStride();
	public:
		BufferLayout() {}
		BufferLayout(const std::initializer_list<BufferElement>& element) : elements(element)
		{
			this->SetOffsetsAndStride();
		}

		inline const std::vector<BufferElement>& GetElements() {
			return this->elements;
		}

		inline uint32_t GetStride() const { return this->stride; }

		std::vector<BufferElement>::iterator begin() { return this->elements.begin(); }
		std::vector<BufferElement>::iterator end() { return this->elements.end(); }
		std::vector<BufferElement>::const_iterator begin() const { return this->elements.begin(); }
		std::vector<BufferElement>::const_iterator end() const { return this->elements.end(); }

	};
	class VertexBuffer
	{
	public:
		virtual ~VertexBuffer() {}
		/// <summary>
		/// Uploads data to the gpu.
		/// </summary>
		/// <param name="pointer">pointer to the beginning of the data</param>
		/// <param name="count">The number of elements in the data</param>
		virtual void SetData(float* pointer, uint32_t count) = 0;
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
		/// Get the buffer layout of this buffer
		/// </summary>
		/// <returns>The layout of the buffer</returns>
		virtual BufferLayout& GetLayout() = 0;
		/// <summary>
		/// Set the alyout of the buffer as defined in memory
		/// </summary>
		/// <param name="layout">The layout of the buffer</param>
		virtual void SetLayout(const BufferLayout& layout) = 0;
		/// <summary>
		/// Creates a RenderAPI specific VertexBuffer
		/// </summary>
		/// <returns>A polymorphic VertexBuffer</returns>
		static VertexBuffer* Create(float* pointer, uint32_t count);
	};
	class IndexBuffer
	{
	public:
		virtual ~IndexBuffer() {}
		/// <summary>
		/// Uploads data to the gpu. Automatically detects the type of the data
		/// </summary>
		/// <param name="pointer">pointer to the beginning of the data</param>
		/// <param name="count">The number of elements in the data</param>
		virtual void SetData(uint32_t* pointer, uint32_t count) = 0;
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
		static IndexBuffer* Create(uint32_t* pointer, uint32_t count);
	};
}
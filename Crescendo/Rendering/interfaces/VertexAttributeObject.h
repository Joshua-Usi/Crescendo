#pragma once

#include "core/core.h"

#include "GraphicsAPI.h"

namespace Crescendo::Rendering
{
	class VertexAttributeObject
	{
	public:
		virtual ~VertexAttributeObject() {}
		
		virtual void SetAttributePointer(uint32_t index, int32_t components, uint64_t size) = 0;
		virtual void EnableAttribute(uint32_t attributeIndex) = 0;
		virtual void DisableAttribute(uint32_t attributeIndex) = 0;
		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		static VertexAttributeObject* Create();
	};
}
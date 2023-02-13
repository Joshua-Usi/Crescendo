#pragma once

#include "core/core.h"
#include "interfaces/VertexAttributeObject.h"

namespace Crescendo::Rendering
{
	class OpenGLVertexAttributeObject : public VertexAttributeObject
	{
	private:
		uint32_t vertexAttributeID;
	public:
		OpenGLVertexAttributeObject();
		virtual ~OpenGLVertexAttributeObject();
		
		virtual void SetAttributePointer(uint32_t index, int32_t components, uint64_t size) override;
		virtual void EnableAttribute(uint32_t attributeIndex) override;
		virtual void DisableAttribute(uint32_t attributeIndex) override;
		virtual void Bind() override;
		virtual void Unbind() override;
	};
}
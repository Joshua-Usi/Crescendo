#pragma once

#include "core/core.h"
#include "XMLNode.h"

namespace Crescendo::Tools::XML
{
	class Document : public AttributeContainer
	{
	public:
		std::unique_ptr<Node> root;
	public:
		Document()
		{
			root.reset(new Node);
		}
		Node* GetRootNode()
		{
			return root.get();
		}
	};
}
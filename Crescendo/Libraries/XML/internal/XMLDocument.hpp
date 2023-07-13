#pragma once

#include "XMLNode.hpp"

namespace Crescendo::Tools::XML
{
	class Document : public AttributeContainer
	{
	public:
		unique<Node> root;
	public:
		Document()
		{
			root.reset(new Node);
		}
		Node* GetRootNode() const
		{
			return root.get();
		}
	};
}
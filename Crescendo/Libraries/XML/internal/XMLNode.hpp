#pragma once

#include <vector>

#include "Core/common.hpp"
#include "XMLAttributeContainer.hpp"

namespace Crescendo::Tools::XML
{
	// Provides a javascript-like interface to nodes
	class Node : public AttributeContainer
	{
	public:
		std::vector<unique<Node>> children;
		Node* parent;
		std::string tag;
		std::string innerText;
	public:
		// RapidXML like syntax
		inline Node* _first_node() const
		{
			if (this->GetChildCount() > 0) return this->GetChild(0);
			return nullptr;
		}
		inline Node* _next_sibling()
		{
			if (!this->GetParent()) return nullptr;
			unsigned int i = 0;
			while (i < this->GetParent()->GetChildCount())
			{
				if ((this->GetParent()->children[i].get() == this) && i + 1 < this->GetParent()->GetChildCount())
				{
					return this->GetParent()->GetChild(i + 1);
				}
				i++;
			}
			return nullptr;
		}
		inline Node* _parent()
		{
			return (this->GetParent()) ? this->GetParent() : nullptr;
		}
		inline Node(Node* par = nullptr)
		{
			parent = par;
			if (parent != nullptr) parent->children.emplace_back(this);
		}
		/// <summary>
		/// Returns a pointer to the parent nodes of the current node
		/// </summary>
		/// <returns>Pointer to parent node</returns>
		inline Node* GetParent() const
		{
			return parent;
		}
		/// <summary>
		/// Returns a pointer that points to the beginning of the children array
		/// </summary>
		/// <returns>Pointer to beginning of children array</returns>
		inline Node** GetChildren() const
		{
			return (Node**)children.data()->get();
		}
		/// <summary>
		/// Gets a specific child by its index, if the index is out of range, it returns a null pointer
		/// </summary>
		/// <param name="n">Index of the child</param>
		/// <returns>Pointer to child</returns>
		inline Node* GetChild(unsigned int n) const
		{
			if (n < children.size())
			{
				return children[n].get();
			}
			return nullptr;
		}
		/// <summary>
		/// Returns the number of child nodes the current node has
		/// </summary>
		/// <returns>The number of child nodes as an integer</returns>
		inline uint64_t GetChildCount() const
		{
			return children.size();
		}
		/// <summary>
		/// Appends a number of nodes to the current node
		/// </summary>
		/// <param name="nodes">A set of nodes to append</param>
		template <typename...>
		inline void Append(Node* nodes, ...)
		{
			for (const auto node : { nodes... }) {
				node->parent = this;
				children.push_back(node);
			}
		}
		/// <summary>
		/// Appends a singular node to the current node
		/// </summary>
		/// <param name="node">Pointer to node</param>
		inline void AppendChild(Node* node)
		{
			node->parent = this;
			children.emplace_back(node);
		}
		/// <summary>
		/// Removes a child node from the current node
		/// </summary>
		/// <param name="n">index of the child node to remove</param>
		inline void RemoveChild(unsigned int n)
		{
			children.erase(children.begin() + n);
		}
		/// <summary>
		/// Removes all child nodes from the current node
		/// </summary>
		inline void RemoveAllChildren()
		{
			children.clear();
		}
		/// <summary>
		/// Gets the tag name of the node
		/// </summary>
		/// <returns>Tag name of the node</returns>
		inline std::string GetTagName() const
		{
			return tag;
		}
		/// <summary>
		/// Gets the text content of the node
		/// </summary>
		/// <returns>Text content of the node</returns>
		inline std::string GetTextContent() const
		{
			return innerText;
		}
	};
}
#pragma once

#include <vector>
#include <memory>

#include "core/core.h"
#include "XMLAttributeContainer.h"

namespace Crescendo::Tools::XML
{
	// Provides a javascript-like interface to nodes
	class Node : public AttributeContainer
	{
	public:
		std::vector<std::unique_ptr<Node>> children;
		Node* parent;
		gt::string tag;
		gt::string innerText;
	public:
		Node(Node* par = nullptr)
		{
			parent = par;
			if (parent != nullptr)
			{
				parent->children.emplace_back(this);
			}
		}
		/// <summary>
		/// Returns a pointer to the parent nodes of the current node
		/// </summary>
		/// <returns>Pointer to parent node</returns>
		Node* GetParent()
		{
			return parent;
		}
		/// <summary>
		/// Returns a pointer that points to the beginning of the children array
		/// </summary>
		/// <returns>Pointer to beginning of children array</returns>
		Node** GetChildren()
		{
			return (Node**)children.data()->get();
		}
		/// <summary>
		/// Gets a specific child by its index, if the index is out of range, it returns a null pointer
		/// </summary>
		/// <param name="n">Index of the child</param>
		/// <returns>Pointer to child</returns>
		Node* GetChild(unsigned int n)
		{
			if (children.size() < n)
			{
				return children[n].get();
			}
			return nullptr;
		}
		/// <summary>
		/// Returns the number of child nodes the current node has
		/// </summary>
		/// <returns>The number of child nodes as an integer</returns>
		gt::Uint64 GetChildCount()
		{
			return children.size();
		}
		/// <summary>
		/// Appends a number of nodes to the current node
		/// </summary>
		/// <param name="nodes">A set of nodes to append</param>
		template <typename...>
		void Append(Node* nodes, ...)
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
		void AppendChild(Node* node)
		{
			node->parent = this;
			children.emplace_back(node);
		}
		/// <summary>
		/// Removes a child node from the current node
		/// </summary>
		/// <param name="n">index of the child node to remove</param>
		void RemoveChild(unsigned int n)
		{
			children.erase(children.begin() + n);
		}
		/// <summary>
		/// Removes all child nodes from the current node
		/// </summary>
		void RemoveAllChildren()
		{
			children.clear();
		}
		/// <summary>
		/// Gets the tag name of the node
		/// </summary>
		/// <returns>Tag name of the node</returns>
		gt::string GetTagName()
		{
			return tag;
		}
		/// <summary>
		/// Gets the text content of the node
		/// </summary>
		/// <returns>Text content of the node</returns>
		gt::string GetTextContent()
		{
			return innerText;
		}
	};
}
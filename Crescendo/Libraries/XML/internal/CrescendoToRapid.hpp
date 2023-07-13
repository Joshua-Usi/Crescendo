#pragma once

#include "../XML.hpp"

#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_ext.hpp"
#include "rapidxml/rapidxml_print.hpp"

namespace Crescendo::Tools::XML::internal
{
	inline rapidxml::xml_node<>* ConvertCrescendoNodeToRapid(rapidxml::xml_document<char>* rapidDoc, Node* csNode)
	{
		char* tag = rapidDoc->allocate_string(csNode->GetTagName().c_str());
		char* text = rapidDoc->allocate_string(csNode->GetTextContent().c_str());
		rapidxml::xml_node<>* node = rapidDoc->allocate_node(rapidxml::node_element, tag, text);
		for (auto const& attribute : csNode->attributes)
		{
			char* key = rapidDoc->allocate_string(attribute.first);
			char* value = rapidDoc->allocate_string(attribute.second);
			rapidxml::xml_attribute<>* rapidAttribute = rapidDoc->allocate_attribute(key, value);
			node->append_attribute(rapidAttribute);
		}
		return node;
	};
	inline void CrescendoToRapid(const Document& crescendoDoc, rapidxml::xml_document<char>* rapidDoc)
	{
		Node* csWorkingNode = crescendoDoc.root.get();
		rapidxml::xml_node<>* workingNode = rapidDoc;

		// Move declaration node, if declarations exist
		if (crescendoDoc.AttributeCount() > 0)
		{
			rapidxml::xml_node<>* declarationNode = rapidDoc->allocate_node(rapidxml::node_declaration);
			// Move individual declarations over
			for (auto const& attribute : crescendoDoc.attributes)
			{
				rapidxml::xml_attribute<>* rapidAttribute = rapidDoc->allocate_attribute(attribute.first, attribute.second);
				declarationNode->append_attribute(rapidAttribute);
			}
			// Append declaration node
			rapidDoc->append_node(declarationNode);
		}
		
		bool wasLastOperationUp = false;

		// The below loop for some reason skips the root node
		workingNode = ConvertCrescendoNodeToRapid(rapidDoc, csWorkingNode);
		rapidDoc->append_node(workingNode);

		do
		{
			// depth first search
			// if there is a child node and we haven't recently moved up a node, take it
			// if there isn't a child node, move up the tree and go to the sibling node
			// otherwise, move back up a node
			// this ensures all nodes are reached
			if (!wasLastOperationUp && csWorkingNode->_first_node())
			{
				wasLastOperationUp = false;

				csWorkingNode = csWorkingNode->_first_node();
				rapidxml::xml_node<>* node = ConvertCrescendoNodeToRapid(rapidDoc, csWorkingNode);

				workingNode->append_node(node);
				workingNode = node;
			}
			else if (csWorkingNode->_next_sibling())
			{
				wasLastOperationUp = false;

				csWorkingNode = csWorkingNode->_next_sibling();
				rapidxml::xml_node<>* node = ConvertCrescendoNodeToRapid(rapidDoc, csWorkingNode);

				workingNode->parent()->append_node(node);
				workingNode = node;
			}
			else {
				wasLastOperationUp = true;

				csWorkingNode = csWorkingNode->_parent();
				workingNode = workingNode->parent();
			}
		} while (csWorkingNode != crescendoDoc.root.get());
	};
}
#pragma once

#include "xml.hpp"

#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_ext.hpp"
#include "rapidxml/rapidxml_print.hpp"

namespace cs_std::xml::internal
{
	rapidxml::xml_node<>* cs_std_node_to_rapid_node(rapidxml::xml_document<char>* rapidDoc, node* csNode)
	{
		char* tag = rapidDoc->allocate_string(csNode->tag.c_str());
		char* text = rapidDoc->allocate_string(csNode->innerText.c_str());
		rapidxml::xml_node<>* node = rapidDoc->allocate_node(rapidxml::node_element, tag, text);
		for (auto const& [name, text] : csNode->attributes)
		{
			char* key = rapidDoc->allocate_string(name.c_str());
			char* value = rapidDoc->allocate_string(text.c_str());
			rapidxml::xml_attribute<>* rapidAttribute = rapidDoc->allocate_attribute(key, value);
			node->append_attribute(rapidAttribute);
		}
		return node;
	};
	void cs_std_to_rapid(const document& crescendoDoc, rapidxml::xml_document<char>* rapidDoc)
	{
		node* csWorkingNode = crescendoDoc.root.get();
		rapidxml::xml_node<>* workingNode = rapidDoc;

		if (crescendoDoc.attribute_count() > 0)
		{
			rapidxml::xml_node<>* declarationNode = rapidDoc->allocate_node(rapidxml::node_declaration);
			for (auto const& [name, text] : crescendoDoc.attributes)
			{
				rapidxml::xml_attribute<>* rapidAttribute = rapidDoc->allocate_attribute(name.c_str(), text.c_str());
				declarationNode->append_attribute(rapidAttribute);
			}
			rapidDoc->append_node(declarationNode);
		}
		
		bool wasLastOperationUp = false;

		// The below loop for some reason skips the root node
		workingNode = cs_std_node_to_rapid_node(rapidDoc, csWorkingNode);
		rapidDoc->append_node(workingNode);

		// Lol do while go br
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
				rapidxml::xml_node<>* node = cs_std_node_to_rapid_node(rapidDoc, csWorkingNode);

				workingNode->append_node(node);
				workingNode = node;
			}
			else if (csWorkingNode->_next_sibling())
			{
				wasLastOperationUp = false;

				csWorkingNode = csWorkingNode->_next_sibling();
				rapidxml::xml_node<>* node = cs_std_node_to_rapid_node(rapidDoc, csWorkingNode);

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
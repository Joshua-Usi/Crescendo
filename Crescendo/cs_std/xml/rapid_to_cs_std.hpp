#pragma once

#include "xml.hpp"

#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_ext.hpp"
#include "rapidxml/rapidxml_print.hpp"

namespace cs_std::xml::internal
{
	void rapid_to_cs_std(document& crescendoDoc, rapidxml::xml_document<char>* rapidDoc)
	{
		crescendoDoc.root = std::make_unique<node>(nullptr, "");
		node* csWorkingNode = crescendoDoc.root.get();
		rapidxml::xml_node<>* workingNode = rapidDoc->first_node();

		// Move declaration node, if declarations exist
		if (workingNode->name_size() == 0)
		{
			for (rapidxml::xml_attribute<>* attr = workingNode->first_attribute(); attr; attr = attr->next_attribute())
			{
				crescendoDoc.attributes[attr->name()] = attr->value();
			}
			// Move to root node
			workingNode = workingNode->next_sibling();
		}

		// Note the root node
		rapidxml::xml_node<>* rootNode = workingNode;

		bool wasLastOperationUp = false;

		do
		{
			if (!wasLastOperationUp)
			{
				csWorkingNode->tag = workingNode->name();
				csWorkingNode->innerText = workingNode->value();
				// Assign RapidXML attributes to CrescendoXML document attributes
				for (rapidxml::xml_attribute<>* attr = workingNode->first_attribute(); attr; attr = attr->next_attribute())
				{
					csWorkingNode->attributes[attr->name()] = attr->value();
				}
			}
			// depth first search
			// if there is a child node and we haven't recently moved up a node, take it
			// if there isn't a child node, move up the tree and go to the sibling node
			// otherwise, move back up a node
			// this ensures all nodes are reached
			if (!wasLastOperationUp && workingNode->first_node())
			{
				wasLastOperationUp = false;

				workingNode = workingNode->first_node();
				csWorkingNode = new node(csWorkingNode, "");
			}
			else if (workingNode->next_sibling())
			{
				wasLastOperationUp = false;

				workingNode = workingNode->next_sibling();
				csWorkingNode = new node(csWorkingNode->_parent(), "");
			}
			else
			{
				wasLastOperationUp = true;

				workingNode = workingNode->parent();
				csWorkingNode = csWorkingNode->_parent();
			}
		} while (workingNode != rootNode);
	};
}
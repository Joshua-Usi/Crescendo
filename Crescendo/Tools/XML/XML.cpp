#include "XML.h"

#include "filesystem/filesystem.h"
#include "filesystem/synchronous/syncFiles.h"

#include "rapidxml/rapidxml.hpp"

#include <memory>

namespace Crescendo::Tools::XML
{
	namespace
	{
		// Convert a RapidXML document to a CrescendoXML document
		void RapidToCrescendo(Document* crescendoDoc, rapidxml::xml_document<char>* rapidDoc)
		{
			Node* csWorkingNode = crescendoDoc->root.get();
			rapidxml::xml_node<>* workingNode = rapidDoc->first_node();

			// in declaration, set document attributes
			if (workingNode->name_size() == 0)
			{
				for (rapidxml::xml_attribute<>* attr = workingNode->first_attribute(); attr; attr = attr->next_attribute())
				{
					crescendoDoc->attributes[attr->name()] = attr->value();
				}
				// Move to root node
				workingNode = workingNode->next_sibling();
			}

			// Note the root node
			rapidxml::xml_node<>* rootNode = nullptr;
			bool wasLastOperationUp = false;

			while (workingNode != rootNode)
			{
				if (!rootNode) rootNode = workingNode;
				// Assign RapidXML node values to CrescendoXML document values
				csWorkingNode->tag = workingNode->name();
				csWorkingNode->innerText = workingNode->value();
				// Assign RapidXML attributes to CrescendoXML document attributes
				for (rapidxml::xml_attribute<>* attr = workingNode->first_attribute(); attr; attr = attr->next_attribute())
				{
					csWorkingNode->attributes[attr->name()] = attr->value();
				}
				// if we can go deeper, go deeper
				// otherwise if we can go to a sibling, we go to a sibling
				// otherwise we go upwards
				// depth first search
				if (!wasLastOperationUp && workingNode->first_node())
				{
					wasLastOperationUp = false;
					workingNode = workingNode->first_node();
					csWorkingNode = new Node(csWorkingNode);
				}
				else if (workingNode->next_sibling())
				{
					wasLastOperationUp = false;
					workingNode = workingNode->next_sibling();

					csWorkingNode = new Node(csWorkingNode->GetParent());
				}
				else
				{
					wasLastOperationUp = true;
					workingNode = workingNode->parent();

					csWorkingNode = csWorkingNode->GetParent();
				}
			}
		};
		void CrescendoToRapid(Document* crescendoDoc, rapidxml::xml_document<char>* rapidDoc)
		{
			// CRESCENDO TODO
		};
	}
	void Parse(Document* xmlDoc, gt::string* xmlString)
	{
		rapidxml::xml_document<char> doc;
		doc.parse<rapidxml::parse_declaration_node>(xmlString->data());
		RapidToCrescendo(xmlDoc, &doc);
	}
	void ParseFromFile(Document* xmlDoc, gt::string filePath)
	{
		gt::file file;
		std::stringstream data;
		// Check if file exists first
		if (!Crescendo::Engine::FileSystem::Exists(filePath)) return;
		Crescendo::Engine::FileSystem::Open(&file, filePath);
		// Write data to stringstream buffer
		data << file.rdbuf();
		std::string d = data.str();
		Parse(xmlDoc, &d);
		Crescendo::Engine::FileSystem::Close(&file);
	}
}
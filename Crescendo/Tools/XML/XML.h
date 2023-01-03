#pragma once

#include <vector>
#include <map>

#include "core/core.h"

// This isn't a full parser!!! Yet
// XML parser designed for converted xml documents to runtime windows
namespace Crescendo::Tools::XML
{
	class XMLNode
	{
	public:
		XMLNode* parent;
		// <tag attributes=attribute>innerText</tag>
		std::string tag;
		std::string innerText;
		std::vector<XMLNode*> children;
		std::map<std::string, std::string> attributes;
	public:
		XMLNode(XMLNode* par)
		{
			parent = par;
			if (parent != nullptr)
			{
				parent->children.push_back(this);
			}
		}
	};
	class XMLDocument
	{
	public:
		XMLNode* root = new XMLNode(nullptr);
		//void Delete(); // TODO
	};

	/// <summary>
	/// Takes a string and parses an xml document
	/// </summary>
	/// <param name="document">document reference</param>
	/// <param name="xml">xml string to parse</param>
	int Parse(XMLDocument* document, std::string xml);
	/// <summary>
	/// Takes a file path and parses the xml document
	/// </summary>
	/// <param name="document">document reference</param>
	/// <param name="filename">path to xml file</param>
	int ParseFromFile(XMLDocument* document, std::string filename);
}
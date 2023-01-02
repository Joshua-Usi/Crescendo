#pragma once

#include <vector>
#include <map>

#include "core/core.h"

// This isn't a full parser!!! Yet
// XML parser designed for converted xml documents to runtime windows
namespace Crescendo::Tools::XML
{
	struct XMLNode
	{
		XMLNode* parent;
		// <tag attributes=attribute>innerText</tag>
		std::string tag;
		std::string innerText;
		std::map<std::string, std::string> attributes;
	};
	class XMLDocument
	{
	public:
		XMLNode* root = new XMLNode;
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
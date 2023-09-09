#include "XML.hpp"

#include "Core/include/File.hpp"

#include "internal/CrescendoToRapid.hpp"
#include "internal/RapidToCrescendo.hpp"

#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_ext.hpp"
#include "rapidxml/rapidxml_print.hpp"

#include <vector>
#include <sstream>

namespace Crescendo::Tools::XML
{
	Document Parse(const std::string& xmlString)
	{
		Document xmlDoc;

		rapidxml::xml_document<char> doc;
		doc.parse<rapidxml::parse_declaration_node | rapidxml::parse_no_data_nodes>((char*)xmlString.c_str());
		internal::RapidToCrescendo(xmlDoc, &doc);

		return xmlDoc;
	}
	Document ParseFromFile(const std::string& filePath)
	{
		std::stringstream data;
		TextFile file(filePath);
		if (!file.Exists()) CS_ASSERT(false, "File does not exist!");
		file.Open();
		data << file.Read();
		return Parse(data.str().c_str());
	}
	std::string Stringify(const Document& xmlDoc)
	{
		std::string outputString;

		rapidxml::xml_document<char> doc;
		internal::CrescendoToRapid(xmlDoc, &doc);
		rapidxml::print(&outputString, doc, 0);

		return outputString;
	}
}
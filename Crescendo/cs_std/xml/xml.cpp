#include "xml.hpp"

#include "../file.hpp"

#include "cs_std_to_rapid.hpp"
#include "rapid_to_cs_std.hpp"

#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_ext.hpp"
#include "rapidxml/rapidxml_print.hpp"

namespace cs_std::xml
{
	document parse(const std::string& xmlString)
	{
		document xmlDoc {};

		rapidxml::xml_document<char> doc {};
		doc.parse<rapidxml::parse_declaration_node | rapidxml::parse_no_data_nodes>((char*)xmlString.c_str());
		internal::rapid_to_cs_std(xmlDoc, &doc);

		return xmlDoc;
	}
	document parse_file(const std::string& filePath)
	{
		std::stringstream data {};
		return parse(cs_std::text_file(filePath).open().read());
	}
	std::string stringify(const document& xmlDoc)
	{
		std::string outputString {};

		rapidxml::xml_document<char> doc;
		internal::cs_std_to_rapid(xmlDoc, &doc);
		rapidxml::print(&outputString, doc, 0);

		return outputString;
	}
}
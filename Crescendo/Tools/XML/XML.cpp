#include "XML.h"

#include "filesystem/filesystem.h"
#include "filesystem/synchronous/syncFiles.h"
#include "console/console.h"

#include "converters/CrescendoToRapid.h"
#include "converters/RapidToCrescendo.h"

#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_ext.hpp"
#include "rapidxml/rapidxml_print.hpp"

#include <vector>
#include <stack>

namespace Crescendo::Tools::XML
{
	void Parse(Document* xmlDoc, gt::string* xmlString)
	{
		rapidxml::xml_document<char>* doc = new rapidxml::xml_document<char>;
		doc->parse<rapidxml::parse_declaration_node | rapidxml::parse_no_data_nodes>(xmlString->data());
		util::RapidToCrescendo(xmlDoc, doc);
		delete doc;
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
	void Stringify(Document* xmlDoc, gt::string* outputString)
	{
		rapidxml::xml_document<char>* doc = new rapidxml::xml_document<char>;
		util::CrescendoToRapid(xmlDoc, doc);
		rapidxml::print(std::back_inserter(*outputString), *doc, 0);
		delete doc;
	}
}
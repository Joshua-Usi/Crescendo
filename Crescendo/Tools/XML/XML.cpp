#include "XML.h"

#include "filesystem/filesystem.h"
#include "filesystem/synchronous/syncFiles.h"

#include "converters/CrescendoToRapid.h"
#include "converters/RapidToCrescendo.h"

#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_ext.hpp"
#include "rapidxml/rapidxml_print.hpp"

#include <vector>

namespace Crescendo::Tools::XML
{
	void Parse(Document& xmlDoc, const char* xmlString)
	{
		// Heap allocate as this takes up quite a bit of stack space
		rapidxml::xml_document<char>* doc = new rapidxml::xml_document<char>;
		doc->parse<rapidxml::parse_declaration_node | rapidxml::parse_no_data_nodes>((char*)xmlString);
		util::RapidToCrescendo(xmlDoc, doc);
		delete doc;
	}
	void ParseFromFile(Document& xmlDoc, const char* filePath)
	{
		std::fstream file;
		std::stringstream data;
		// Check if file exists first
		if (!Crescendo::Engine::FileSystem::Exists(filePath)) return;
		Crescendo::Engine::FileSystem::Open(file, filePath);
		// Write data to stringstream buffer
		data << file.rdbuf();
		Parse(xmlDoc, data.str().c_str());
		Crescendo::Engine::FileSystem::Close(file);
	}
	void Stringify(Document& xmlDoc, std::string& outputString)
	{
		// Heap allocate as this takes up quite a bit of stack space
		rapidxml::xml_document<char>* doc = new rapidxml::xml_document<char>;
		util::CrescendoToRapid(xmlDoc, doc);
		rapidxml::print(&outputString, *doc, 0);
		delete doc;
	}
}
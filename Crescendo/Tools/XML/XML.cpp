/*
 *	Special Thanks to: jonahisadev
 *	This parser is heavily based on their version
 *	https://www.youtube.com/watch?v=kPFYfTvMRs8
 */	

#include <cstdlib>

#include "XML.h"
#include "filesystem/filesystem.h"
#include "filesystem/synchronous/syncFiles.h"

#include "console/console.h"

#define XML_ERR_MISMATCHED_TAGS -3
#define XML_ERR_END_TAG_SPECIFIED_BEFORE_START -2
#define XML_ERR_FILE_NOT_FOUND -1
#define XML_SUCCESS 0

namespace Crescendo::Tools::XML
{
	int Parse(XMLDocument* document, std::string* xml)
	{
		std::string source = *xml;
		struct {
			// Tag buffer
			std::stringstream tag;
			// Inner text buffer
			std::stringstream text;
			// Attribute tag buffer
			std::stringstream attributeTag;
			// Attribute inner text buffer
			std::stringstream attributeText;
		} lexicalBuffer;
		cs::uint64 i = 0;

		XMLNode* currentNode = nullptr;
		// Lexical analysis
		while (i < source.size())
		{
			// Start of tag
			if (source[i] == '<')
			{
				// Add inner text if found
				if (currentNode && lexicalBuffer.text.str().size() > 0)
				{	
					currentNode->innerText.assign(lexicalBuffer.text.str());
					lexicalBuffer.text.str(std::string());
					lexicalBuffer.text.clear();
				}
				// Check for start of end tag, denoted by / <root></root>
				if (source[i + 1] == '/')
				{
					// Jump to character directly after the /
					i += 2;
					// Read begin tag until end character is found
					while (source[i] != '>')
					{
						lexicalBuffer.tag << source[i];
						i++;
					}

					// If a node isn't specified, then the end tag is the root tag and the first tag in the document
					if (!currentNode) {
						return XML_ERR_END_TAG_SPECIFIED_BEFORE_START;
					}

					// If the current nodes tag is different to the tag of the end tag, then the tags are mismatched
					if (currentNode->tag.compare(lexicalBuffer.text.str()) != 0)
					{
						return XML_ERR_MISMATCHED_TAGS;
					}
					currentNode = currentNode->parent;
					i++;
					continue;
				}
				// Check if the node is the root node or not, 
				if (!currentNode)
				{
					currentNode = document->root;
				}
				else
				{
					currentNode = new XMLNode;
				}
				i++;
				// Read start tag until " " or ">"
				// If it is a space, then it denotes there are some attributes
				// If it is a >, then it denotes it is the beginning of the inner text
				while (source[i] != '>')
				{
					lexicalBuffer.tag << source[i];
					i++;
					// Denotes that at least one or more attributes exist
					if (source[i] == ' ')
					{
						i++;
						// Keep looping till the end character is reached
						while (source[i] != '>')
						{
							// Continue looping until an "=" is found,
							// This loop finds the attribute tag
							while (source[i] != '=')
							{
								lexicalBuffer.attributeTag << source[i];
								i++;
							}
							// Skip one character (skips ")
							i += 2;
							// Continue looping until an " is found
							// This loop finds the attribute value
							while (source[i] != '\"')
							{
								lexicalBuffer.attributeText<< source[i];
								i++;
							}
							i++;
							// Insert attribute tags
							currentNode->attributes[lexicalBuffer.attributeTag.str()] = lexicalBuffer.attributeText.str();
							// Clear tags for next loop
							lexicalBuffer.attributeTag.str(std::string());
							lexicalBuffer.attributeTag.clear();
							lexicalBuffer.attributeText.str(std::string());
							lexicalBuffer.attributeText.clear();
						}
					}
				}
				// Insert tag into lexicalbuffer;
				currentNode->tag.assign(lexicalBuffer.tag.str());
				// Clear tag for next loop
				lexicalBuffer.tag.str(std::string());
				lexicalBuffer.tag.clear();
				// move on to next character >
				i++;
			}
			else {
				// Insert innertext characters
				lexicalBuffer.text << source[i];
				i++;
			}
		}
		return XML_SUCCESS;
	}
	int ParseFromFile(XMLDocument* document, std::string filename)
	{
		std::fstream file;
		std::stringstream data;
		// Check if file exists first
		if (!Engine::FileSystem::Exists(filename))
		{
			return XML_ERR_FILE_NOT_FOUND;
		}
		Engine::FileSystem::Open(&file, filename);
		// Write data to stringstreambuffer
		data << file.rdbuf();
		std::string str = data.str();
		return Parse(document, &str);
	}
}
/*
 *	Special Thanks to: jonahisadev
 *	This parser is heavily based on their version
 *	https://www.youtube.com/watch?v=kPFYfTvMRs8
 */	

#include <cstdlib>

#include "XML.h"
#include "filesystem/filesystem.h"
#include "filesystem/synchronous/syncFiles.h"

namespace Crescendo::Tools::XML
{
	namespace {
		bool IsSubstring(std::string string, std::string substring, cs::uint64 index)
		{
			for (cs::uint64 i = index, j = 0; i < index + substring.size(); i++, j++)
			{
				if (string[i] != substring[j])
				{
					return false;
				}
			}
			return true;
		}
		std::string StripComments(std::string* xml)
		{
			bool inComment = true;
			std::string source = *xml;
			cs::uint64 i = 0;
			while (i < source.size())
			{
				// Find start of comment
				if (IsSubstring(source, "<!--", i))
				{
					// Note start of comment
					cs::uint64 j = i;
					// Find end of comment
					while (!IsSubstring(source, "-->", i)) {
						i++;
					}
					// Replace entire substring of comment with empty string
					// +3 to include the end tag of a comment
					source.replace(j, i - j + 3, "");
					i = j;
				}
				else
				{
					i++;
				}
			}
			return source;
		}
	}
	XMLStatus Parse(XMLDocument* document, std::string* xml)
	{
		std::string source = StripComments(xml);
		std::cout << source << std::endl;
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
		// Strip Comments
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
				// Check for document declarations
				if (IsSubstring(source, "?xml ", i + 1))
				{
					// Skip past declaraton and required space
					i += 5;
					// Keep looping till the end of declaration is found
					while (source[i] != '>' && !IsSubstring(source, "?>", i))
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
							lexicalBuffer.attributeText << source[i];
							i++;
						}
						i++;
						// Insert attribute tags
						document->attributes[lexicalBuffer.attributeTag.str()] = lexicalBuffer.attributeText.str();
						// Clear tags for next loop
						lexicalBuffer.attributeTag.str(std::string());
						lexicalBuffer.attributeTag.clear();
						lexicalBuffer.attributeText.str(std::string());
						lexicalBuffer.attributeText.clear();
					}
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
						return XMLStatus::ErrEndTagsSpecifiedBeforeStart;
					}

					// If the current nodes tag is different to the tag of the end tag, then the tags are mismatched
					if (currentNode->tag.compare(lexicalBuffer.tag.str()) != 0)
					{
						return XMLStatus::ErrMisMatchedTags;
					}
					lexicalBuffer.tag.str(std::string());
					lexicalBuffer.tag.clear();
					currentNode = currentNode->parent;
					i++;
					continue;
				}
				if (!currentNode)
				{
					currentNode = document->root;
				}
				else
				{
					currentNode = new XMLNode(currentNode);
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
		return XMLStatus::Success;
	}
	XMLStatus ParseFromFile(XMLDocument* document, std::string filename)
	{
		std::fstream file;
		std::stringstream data;
		// Check if file exists first
		if (!Engine::FileSystem::Exists(filename))
		{
			return XMLStatus::ErrFileNotFound;
		}
		Engine::FileSystem::Open(&file, filename);
		// Write data to stringstreambuffer
		data << file.rdbuf();
		std::string str = data.str();
		return Parse(document, &str);
	}
}
#include "XML.h"
#include "filesystem/filesystem.h"
#include "filesystem/synchronous/syncFiles.h"

#include "Utils/FiniteStateStack.h"

// TODO CRESCENDO Make XML W3C conforming and fix edge cases
namespace Crescendo::Tools::XML
{
	namespace {
		class LexicalBuffer
		{
		private:
			std::string buffer;
			std::vector<std::string> memoryBuffers;
		public:
			LexicalBuffer(int memoryBufferCount)
			{
				for (int i = 0; i < memoryBufferCount; i++)
				{
					memoryBuffers.push_back(std::string());
				}
			}
			std::string Get()
			{
				return buffer;
			}
			std::string GetStored(int n)
			{
				return memoryBuffers[n];
			}
			void Store(int n)
			{
				memoryBuffers[n] = buffer;
			}
			void ClearStore(int n)
			{
				memoryBuffers[n].clear();
			}
			void Append(char character)
			{
				buffer += character;
			}
			void Clear()
			{
				buffer.clear();
			}
		};
		bool IsSubstring(std::string string, std::string substring, long position)
		{
			// Outside string bounds
			if (position > string.size() - substring.size() || position < 0) return false;
			for (int i = position; i < position + substring.size(); i++)
			{
				if (string[i] != substring[i - position]) return false;
			}
			return true;
		}
		std::string CleanupDocument(std::string* xml)
		{
			bool inComment = false;
			std::string source = *xml;
			long i = 0, j = 0;
			while (i < source.size())
			{
				// Find start of comment
				if (IsSubstring(source, "<!--", i))
				{
					inComment = true;
					// Note start of comment
					j = i;
				}
				if (inComment && IsSubstring(source, "-->", i))
				{
					inComment = false;
					source.replace(j, i - j + 3, "");
					i = j;
				}
				i++;
			}
			return source;
		}
		enum class XMLLexerState
		{
			None,

			Declaration,

			TagBeginName,
			TagInnerText,
			TagEndName,

			AttributeTag,
			AttributeText,
		};
		std::map<XMLLexerState, std::string> stateMap {
			{XMLLexerState::None, "None"},
			{XMLLexerState::Declaration, "Declaration"},
			{XMLLexerState::TagBeginName, "TagBeginName"},
			{XMLLexerState::TagInnerText, "TagInnerText"},
			{XMLLexerState::TagEndName, "TagEndName"},
			{XMLLexerState::AttributeTag, "AttributeTag"},
			{XMLLexerState::AttributeText, "AttributeText"},
		};
	}
	XMLStatus Parse(XMLDocument* document, std::string* xml)
	{
		std::string source = CleanupDocument(xml);
		int i = 0;
		int increment = 1;
		XMLNode* currentNode = nullptr;

		LexicalBuffer lexicalBuffer(2);

		FiniteStateStack<XMLLexerState> lexerState(XMLLexerState::None, stateMap);

		// Lexical analysis
		while (i < source.size())
		{
			lexicalBuffer.Append(source[i]);
			switch (lexerState.GetState())
			{
				case XMLLexerState::None:
				{
					if (IsSubstring(source, "<", i))
					{
						// Declaration start
						if (IsSubstring(source, "<?xml", i))
						{
							lexerState.Push(XMLLexerState::Declaration);
							increment = 4;
						}
						// Otherwise its a node
						else
						{
							lexerState.Push(XMLLexerState::TagBeginName);
						}
						lexicalBuffer.Clear();
					}
					break;
				}
				case XMLLexerState::Declaration:
				{
					if (IsSubstring(source, "?>", i + 1))
					{
						lexerState.Pop();
						increment = 3;
					}
					else
					{
						lexerState.Push(XMLLexerState::AttributeTag);
						increment = 2;
					}
					lexicalBuffer.Clear();
					break;
				}
				case XMLLexerState::TagBeginName:
				{
					if (IsSubstring(source, ">", i + 1) || IsSubstring(source, " ", i + 1))
					{
						if (lexicalBuffer.GetStored(0).size() == 0)
						{
							currentNode = (currentNode) ? new XMLNode(currentNode) : document->root;
							lexicalBuffer.Store(0);
						}
						lexicalBuffer.Clear();
						increment = 2;
					}
					if (IsSubstring(source, ">", i + 1))
					{
						lexerState.Push(XMLLexerState::TagInnerText);
						std::string tag = lexicalBuffer.GetStored(0);
						currentNode->tag = tag;
						lexicalBuffer.ClearStore(0);
					}
					else if (IsSubstring(source, " ", i + 1))
					{
						lexerState.Push(XMLLexerState::AttributeTag);
					}
					break;
				}
				case XMLLexerState::TagInnerText:
				{
					if (IsSubstring(source, "</", i + 1))
					{
						currentNode->innerText = lexicalBuffer.Get();
						lexerState.Push(XMLLexerState::TagEndName);
						lexicalBuffer.Clear();
						increment = 3;
					}
					else if (IsSubstring(source, "<", i + 1))
					{
						lexerState.Push(XMLLexerState::TagBeginName);
						lexicalBuffer.Clear();
						increment = 2;
					}
					break;
				}
				case XMLLexerState::TagEndName:
				{
					if (IsSubstring(source, ">", i + 1))
					{
						if (currentNode->tag != lexicalBuffer.Get())
						{
							return XMLStatus::ErrorMismatchedTags;
						}
						lexerState.Pop(3);
						lexicalBuffer.Clear();
						currentNode = currentNode->parent;
					}
					break;
				}
				case XMLLexerState::AttributeTag:
				{
					if (IsSubstring(source, "=\"", i + 1))
					{
						lexerState.Push(XMLLexerState::AttributeText);
						lexicalBuffer.Store(1);
						lexicalBuffer.Clear();
						increment = 3;
					}
					break;
				}
				case XMLLexerState::AttributeText:
				{
					if (IsSubstring(source, "\"", i + 1))
					{
						// In declaration
						if (lexerState.StateExists(XMLLexerState::Declaration))
						{
							document->attributes[lexicalBuffer.GetStored(1)] = lexicalBuffer.Get();
						}
						// In node
						else
						{
							currentNode->attributes[lexicalBuffer.GetStored(1)] = lexicalBuffer.Get();
						}
						lexerState.Pop(2);
						lexicalBuffer.Clear();
					}
					break;
				}
			}
			i += increment;
			increment = 1;
		}
		if (lexerState.GetState() != XMLLexerState::None)
		{
			return XMLStatus::ErrorUnclosedTag;
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
			return XMLStatus::Success;
		}
		Engine::FileSystem::Open(&file, filename);
		// Write data to stringstreambuffer
		data << file.rdbuf();
		std::string str = data.str();
		return Parse(document, &str);
	}
}
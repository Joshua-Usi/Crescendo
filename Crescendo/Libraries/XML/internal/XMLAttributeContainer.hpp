#pragma once

#include <map>
#include <string>

namespace Crescendo::Tools::XML
{
	class AttributeContainer
	{
	public:
		std::map<const char*, const char*> attributes;
	public:
		inline std::string GetAttribute(const char* attributeName)
		{
			return attributes[attributeName];
		}
		inline void SetAttribute(const char* attributeName, const char* attributeValue)
		{
			attributes[attributeName] = attributeValue;
		}
		inline void RemoveAttribute(const char* attributeName)
		{
			attributes.erase(attributeName);
		}
		inline uint64_t AttributeCount() const
		{
			return attributes.size();
		}
	};
}
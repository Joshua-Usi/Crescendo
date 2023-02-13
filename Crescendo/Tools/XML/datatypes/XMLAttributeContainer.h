#pragma once

#include <map>

#include "core/core.h"

namespace Crescendo::Tools::XML
{
	class AttributeContainer
	{
	public:
		std::map<const char*, const char*> attributes;
	public:
		std::string GetAttribute(const char* attributeName)
		{
			return attributes[attributeName];
		}
		void SetAttribute(const char* attributeName, const char* attributeValue)
		{
			attributes[attributeName] = attributeValue;
		}
		void RemoveAttribute(const char* attributeName)
		{
			attributes.erase(attributeName);
		}
		uint64_t AttributeCount()
		{
			return attributes.size();
		}
	};
}
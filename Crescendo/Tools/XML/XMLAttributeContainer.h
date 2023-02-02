#pragma once

#include <map>

#include "core/core.h"

namespace Crescendo::Tools::XML
{
	class AttributeContainer
	{
	public:
		std::map<gt::string, gt::string> attributes;
	public:
		gt::string GetAttribute(gt::string attributeName)
		{
			return attributes[attributeName].data();
		}
		void SetAttribute(gt::string attributeName, gt::string attributeValue)
		{
			attributes[attributeName] = attributeValue;
		}
		void RemoveAttribute(gt::string attributeName)
		{
			attributes.erase(attributeName);
		}
		gt::Int64 AttributeCount()
		{
			return attributes.size();
		}
	};
}
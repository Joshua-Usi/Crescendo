#pragma once

#include <cstdint>
#include <vector>
#include <algorithm>

namespace cs_std::graphics
{
	/// <summary>
	/// Defines attribute flags for a mesh
	/// Based off the minimum required attributes for a gltf compliant renderer
	/// </summary>
	enum class Attribute: uint32_t
	{
		POSITION = 0,
		NORMAL,
		TANGENT,
		TEXCOORD_0,
		TEXCOORD_1,
		COLOR_0,
		JOINTS_0,
		WEIGHTS_0,
		UNKNOWN,
		// Must be last
		ATTRIBUTE_COUNT
	};

	// Specifies the data for a single attribute
	struct shader_attribute
	{
		std::vector<float> data;
		Attribute attribute;

		// Comparator for sorting
		bool operator<(const shader_attribute& other) const {
			return attribute < other.attribute;
		}
	};

	// Specifies the data for a mesh
	struct mesh
	{
		std::vector<shader_attribute> attributes;
		std::vector<uint32_t> indices;

		inline const shader_attribute& get_attribute(Attribute attribute) const
		{
			for (const shader_attribute& attr : this->attributes)
			{
				if (attr.attribute == attribute) return attr;
			}
			return this->attributes[0];
		}
		inline shader_attribute& get_attribute(Attribute attribute)
		{
			for (shader_attribute& attr : this->attributes)
			{
				if (attr.attribute == attribute) return attr;
			}
			return this->attributes[0];
		}
		inline bool has_attribute(Attribute attribute) const
		{
			for (const shader_attribute& attr : this->attributes)
			{
				if (attr.attribute == attribute) return true;
			}
			return false;
		}
		// Insert in-place according to Attribute index
		inline void add_attribute(Attribute attribute, const std::vector<float>& data)
		{
			this->attributes.push_back({ data, attribute });
			std::sort(this->attributes.begin(), this->attributes.end());
		}
	};
}
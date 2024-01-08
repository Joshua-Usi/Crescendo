#pragma once

#include "common.hpp"

#include "Volk/volk.h"

#include <algorithm>

#define CS_DEFINE_ENUM_CLASS_OR_OPERATOR(EnumType, EnumName)\
	friend EnumName operator|(EnumName a, EnumName b) { return static_cast<EnumName>(static_cast<EnumType>(a) | static_cast<EnumType>(b)); };\
	friend EnumName operator|=(EnumName& a, EnumName b) { return a = a | b; }
#define CS_DEFINE_ENUM_CLASS_AND_OPERATOR(EnumType, EnumName)\
		friend EnumName operator&(EnumName a, EnumName b) { return static_cast<EnumName>(static_cast<EnumType>(a) & static_cast<EnumType>(b)); };\
		friend EnumName operator&=(EnumName& a, EnumName b) { return a = a & b; }
#define CS_DEFINE_ENUM_CLASS_BITWISE_OPERATORS(EnumType, EnumName) CS_DEFINE_ENUM_CLASS_OR_OPERATOR(EnumType, EnumName); CS_DEFINE_ENUM_CLASS_AND_OPERATOR(EnumType, EnumName)

CS_NAMESPACE_BEGIN::Vulkan
{
	struct PipelineVariants
	{
	private:
		static constexpr uint8_t getNthSetBit(uint8_t number, uint8_t n)
		{
			uint8_t count = 0;
			for (uint8_t i = 0; i < 8; i++)
			{
				if (number & (1 << i))
				{
					if (count == n) return (1 << i);
					count++;
				}
			}
			return 0;
		}
	public:
		constexpr static uint8_t FillModeCount = 3, CullModeCount = 3, DepthFuncCount = 7, DepthWriteCount = 2, DepthTestCount = 2;
		enum class FillMode : uint8_t { Solid = 0b1, Wireframe = 0b10, Point = 0b100 };
		enum class CullMode : uint8_t { Back = 0b1, Front = 0b10, None = 0b100 };
		enum class Multisamples : uint8_t { One = 0b1, Two = 0b10, Four = 0b100, Eight = 0b1000, Sixteen = 0b10000, ThirtyTwo = 0b100000, SixtyFour = 0b1000000 };
		enum class DepthFunc : uint8_t { Never = 0b1, Less = 0b10, Equal = 0b100, LessEqual = 0b1000, Greater = 0b10000, GreaterEqual = 0b100000, Always = 0b1000000 };
		enum class DepthTest : uint8_t { Enabled = 0b1, Disabled = 0b10 };
		enum class DepthWrite : uint8_t { Enabled = 0b1, Disabled = 0b10 };
		CS_DEFINE_ENUM_CLASS_BITWISE_OPERATORS(uint8_t, FillMode);
		CS_DEFINE_ENUM_CLASS_BITWISE_OPERATORS(uint8_t, CullMode);
		CS_DEFINE_ENUM_CLASS_BITWISE_OPERATORS(uint8_t, Multisamples);
		CS_DEFINE_ENUM_CLASS_BITWISE_OPERATORS(uint8_t, DepthFunc);
		CS_DEFINE_ENUM_CLASS_BITWISE_OPERATORS(uint8_t, DepthTest);
		CS_DEFINE_ENUM_CLASS_BITWISE_OPERATORS(uint8_t, DepthWrite);
	public:
		VkRenderPass renderPass;
		FillMode fillModeFlags;
		CullMode cullModeFlags;
		Multisamples multisampleFlags;
		DepthFunc depthFuncFlags;
		DepthTest depthTestFlags;
		DepthWrite depthWriteFlags;
	public:
		// Determines the number of pipelines that will be created
		uint32_t GetVariantCount() const
		{
			return
				std::popcount(static_cast<uint8_t>(fillModeFlags)) *
				std::popcount(static_cast<uint8_t>(cullModeFlags)) *
				std::popcount(static_cast<uint8_t>(multisampleFlags)) *
				std::popcount(static_cast<uint8_t>(depthFuncFlags)) *
				std::popcount(static_cast<uint8_t>(depthTestFlags)) *
				std::popcount(static_cast<uint8_t>(depthWriteFlags));
		}
		bool VariantExists(FillMode fillMode, CullMode cullMode, Multisamples multisamples, DepthFunc depthFunc, DepthTest depthTest, DepthWrite depthWrite) const
		{
			return
				static_cast<uint8_t>(fillModeFlags & fillMode) &&
				static_cast<uint8_t>(cullModeFlags & cullMode) &&
				static_cast<uint8_t>(multisampleFlags & multisamples) &&
				static_cast<uint8_t>(depthFuncFlags & depthFunc) &&
				static_cast<uint8_t>(depthTestFlags & depthTest) &&
				static_cast<uint8_t>(depthWriteFlags & depthWrite);
		}
		uint32_t GetVariantIndex(FillMode fillMode, CullMode cullMode, Multisamples multisamples, DepthFunc depthFunc, DepthTest depthTest, DepthWrite depthWrite) const
		{
			// If the variant doesn't exist, return some invalid index, I doubt this will ever be used
			if (!VariantExists(fillMode, cullMode, multisamples, depthFunc, depthTest, depthWrite)) return std::numeric_limits<uint32_t>::max();

			uint32_t index = 0;
			uint32_t multiplier = 1;

			const auto addFlagToIndex = [&](auto flag, auto allFlags)
			{
				uint8_t totalFlags = std::popcount(static_cast<uint8_t>(allFlags));
				uint8_t currentFlagPosition = std::popcount(static_cast<uint8_t>((static_cast<uint8_t>(flag) - 1) & static_cast<uint8_t>(allFlags)));
				index += multiplier * currentFlagPosition;
				multiplier *= totalFlags;
			};

			// Reverse order
			addFlagToIndex(depthWrite, depthWriteFlags);
			addFlagToIndex(depthTest, depthTestFlags);
			addFlagToIndex(depthFunc, depthFuncFlags);
			addFlagToIndex(multisamples, multisampleFlags);
			addFlagToIndex(cullMode, cullModeFlags);
			addFlagToIndex(fillMode, fillModeFlags);

			return index;
		}
		PipelineVariants GetVariant(uint32_t index) const
		{
			PipelineVariants variant{};
			variant.renderPass = renderPass;

			const auto getFlagFromIndex = [&](auto allFlags)
			{
				uint8_t totalFlags = std::popcount(static_cast<uint8_t>(allFlags));
				uint8_t flagIndex = index % totalFlags;
				uint8_t flagValue = 1;
				while (flagIndex > 0)
				{
					flagValue <<= 1;
					if (flagValue & static_cast<uint8_t>(allFlags)) flagIndex--;
				}
				index /= totalFlags;
				return static_cast<uint8_t>(flagValue) - 1;
			};

			// Assign the bit index
			uint8_t depthWriteFlag = getNthSetBit(static_cast<uint8_t>(depthWriteFlags), getFlagFromIndex(depthWriteFlags));
			uint8_t depthTestFlag = getNthSetBit(static_cast<uint8_t>(depthTestFlags), getFlagFromIndex(depthTestFlags));
			uint8_t depthFuncFlag = getNthSetBit(static_cast<uint8_t>(depthFuncFlags), getFlagFromIndex(depthFuncFlags));
			uint8_t multisampleFlag = getNthSetBit(static_cast<uint8_t>(multisampleFlags), getFlagFromIndex(multisampleFlags));
			uint8_t cullModeFlag = getNthSetBit(static_cast<uint8_t>(cullModeFlags), getFlagFromIndex(cullModeFlags));
			uint8_t fillModeFlag = getNthSetBit(static_cast<uint8_t>(fillModeFlags), getFlagFromIndex(fillModeFlags));

			// Note that this function relies on the side effect from getNthSetBit where if the index is invalid, it returns 0;
			variant.depthWriteFlags = (depthWriteFlag) ? static_cast<DepthWrite>(depthWriteFlag) : DepthWrite::Disabled;
			variant.depthTestFlags = (depthTestFlag) ? static_cast<DepthTest>(depthTestFlag) : DepthTest::Disabled;
			variant.depthFuncFlags = (depthFuncFlag) ? static_cast<DepthFunc>(depthFuncFlag) : DepthFunc::Always;
			variant.multisampleFlags = (multisampleFlag) ? static_cast<Multisamples>(multisampleFlag) : Multisamples::SixtyFour;
			variant.cullModeFlags = (cullModeFlag) ? static_cast<CullMode>(cullModeFlag) : CullMode::None;
			variant.fillModeFlags = (fillModeFlag) ? static_cast<FillMode>(fillModeFlag) : FillMode::Point;

			return variant;
		}
	public:
		static VkPolygonMode GetPolygonMode(FillMode fillMode)
		{
			switch (fillMode)
			{
			case FillMode::Solid: return VK_POLYGON_MODE_FILL;
			case FillMode::Wireframe: return VK_POLYGON_MODE_LINE;
			case FillMode::Point: return VK_POLYGON_MODE_POINT;
			}
			// TODO assert or quietly assert
			return VK_POLYGON_MODE_MAX_ENUM;
		}
		static VkCullModeFlags GetCullMode(CullMode cullMode)
		{
			switch (cullMode)
			{
			case CullMode::Back: return VK_CULL_MODE_BACK_BIT;
			case CullMode::Front: return VK_CULL_MODE_FRONT_BIT;
			case CullMode::None: return VK_CULL_MODE_NONE;
			}
			// TODO assert or quietly assert

			return VK_CULL_MODE_FLAG_BITS_MAX_ENUM;
		}
		static VkSampleCountFlagBits GetMultisamples(Multisamples samples)
		{
			switch (samples)
			{
			case Multisamples::One: return VK_SAMPLE_COUNT_1_BIT;
			case Multisamples::Two: return VK_SAMPLE_COUNT_2_BIT;
			case Multisamples::Four: return VK_SAMPLE_COUNT_4_BIT;
			case Multisamples::Eight: return VK_SAMPLE_COUNT_8_BIT;
			case Multisamples::Sixteen: return VK_SAMPLE_COUNT_16_BIT;
			case Multisamples::ThirtyTwo: return VK_SAMPLE_COUNT_32_BIT;
			case Multisamples::SixtyFour: return VK_SAMPLE_COUNT_64_BIT;
			}
			// TODO assert or quietly assert
			return VK_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM;
		}
		static VkCompareOp GetDepthFunc(DepthFunc depthFunc)
		{
			switch (depthFunc)
			{
			case DepthFunc::Never: return VK_COMPARE_OP_NEVER;
			case DepthFunc::Less: return VK_COMPARE_OP_LESS;
			case DepthFunc::Equal: return VK_COMPARE_OP_EQUAL;
			case DepthFunc::LessEqual: return VK_COMPARE_OP_LESS_OR_EQUAL;
			case DepthFunc::Greater: return VK_COMPARE_OP_GREATER;
			case DepthFunc::GreaterEqual: return VK_COMPARE_OP_GREATER_OR_EQUAL;
			case DepthFunc::Always: return VK_COMPARE_OP_ALWAYS;
			}
			// TODO assert or quietly assert
			return VK_COMPARE_OP_MAX_ENUM;
		}
		static VkBool32 GetDepthTest(DepthTest depthTest)
		{
			switch (depthTest)
			{
			case DepthTest::Enabled: return VK_TRUE;
			case DepthTest::Disabled: return VK_FALSE;
			}
			// TODO assert or quietly assert
			return VK_FALSE;
		}
		static VkBool32 GetDepthWrite(DepthWrite depthWrite)
		{
			switch (depthWrite)
			{
			case DepthWrite::Enabled: return VK_TRUE;
			case DepthWrite::Disabled: return VK_FALSE;
			}
			// TODO assert or quietly assert
			return VK_FALSE;
		}
		static Multisamples ConvertSamplesToVariant(uint32_t samples)
		{
			switch (samples)
			{
				case 64: return Multisamples::SixtyFour;
				case 32: return Multisamples::ThirtyTwo;
				case 16: return Multisamples::Sixteen;
				case 8: return Multisamples::Eight;
				case 4: return Multisamples::Four;
				case 2: return Multisamples::Two;
			}
			return Multisamples::One;
		}
	public:
		// Generate optimised pipelines for specific use cases
		// Generates pipelines for double-sided and transparent objects
		static PipelineVariants GetDefaultVariant(VkRenderPass pass, uint32_t samples = 1)
		{
			return PipelineVariants(pass, FillMode::Solid, CullMode::Back | CullMode::None, ConvertSamplesToVariant(samples), DepthFunc::GreaterEqual, DepthTest::Enabled, DepthWrite::Disabled);
		}
		static PipelineVariants GetSkyboxVariant(VkRenderPass pass, uint32_t samples = 1)
		{
			return PipelineVariants(pass, FillMode::Solid, CullMode::Back, ConvertSamplesToVariant(samples), DepthFunc::GreaterEqual, DepthTest::Enabled, DepthWrite::Disabled);
		}
		static PipelineVariants GetShadowVariant(VkRenderPass pass, uint32_t samples = 1)
		{
			return PipelineVariants(pass, FillMode::Solid, CullMode::Front, ConvertSamplesToVariant(samples), DepthFunc::LessEqual, DepthTest::Enabled, DepthWrite::Enabled);
		}
		static PipelineVariants GetDepthPrepassVariant(VkRenderPass pass, uint32_t samples = 1)
		{
			return PipelineVariants(pass, FillMode::Solid, CullMode::Back | CullMode::None, ConvertSamplesToVariant(samples), DepthFunc::GreaterEqual, DepthTest::Enabled, DepthWrite::Enabled);
		}
		static PipelineVariants GetUIVariant(VkRenderPass pass, uint32_t samples = 1)
		{
			return PipelineVariants(pass, FillMode::Solid, CullMode::None, ConvertSamplesToVariant(samples), DepthFunc::Never, DepthTest::Disabled, DepthWrite::Disabled);
		}
		static PipelineVariants GetPostProcessingVariant(VkRenderPass pass, uint32_t samples = 1)
		{
			return PipelineVariants(pass, FillMode::Solid, CullMode::None, ConvertSamplesToVariant(samples), DepthFunc::Always, DepthTest::Disabled, DepthWrite::Disabled);
		}
	public:	
		PipelineVariants operator[](uint32_t index) const { return GetVariant(index); }
	public:
		struct Iterator {
		private:
			const PipelineVariants& pipelineVariants;
			uint32_t currentIndex;
		public:
			Iterator(const PipelineVariants& pv, uint32_t index) : pipelineVariants(pv), currentIndex(index) {}
			Iterator& operator++() { currentIndex++; return *this; }
			bool operator!=(const Iterator& other) const { return currentIndex != other.currentIndex; }
			PipelineVariants operator*() const { return pipelineVariants.GetVariant(currentIndex); }
		};
		Iterator begin() const { return Iterator(*this, 0); }
		Iterator end() const { return Iterator(*this, GetVariantCount()); }
	};
}

#undef CS_DEFINE_ENUM_CLASS_OR_OPERATOR
#undef CS_DEFINE_ENUM_CLASS_AND_OPERATOR
#undef CS_DEFINE_ENUM_CLASS_BITWISE_OPERATORS
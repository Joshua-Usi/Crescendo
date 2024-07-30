#include "PipelineVariants.hpp"

CS_NAMESPACE_BEGIN::Vulkan::Vk
{
	// Finds the value of the nth bit set to 1 in a number
	template <typename T>
	constexpr T getNthSetBit(T number, T n)
	{
		T count = 0;
		for (T i = 0; i < 8 * sizeof(T); i++)
		{
			if (number & (1 << i))
			{
				if (count == n) return (1 << i);
				count++;
			}
		}
		return 0;
	}
	PipelineVariants::PipelineVariants() : flags(0) {}
	PipelineVariants::PipelineVariants(FillMode fillMode, CullMode cullMode, Multisamples multisamples, DepthFunc depthFunc, DepthTest depthTest, DepthWrite depthWrite)
		: fillModeFlags(fillMode), cullModeFlags(cullMode), multisampleFlags(multisamples), depthFuncFlags(depthFunc), depthTestFlags(depthTest), depthWriteFlags(depthWrite) {}
	uint32_t PipelineVariants::GetVariantCount() const
	{
		return
			std::popcount(static_cast<uint8_t>(fillModeFlags)) *
			std::popcount(static_cast<uint8_t>(cullModeFlags)) *
			std::popcount(static_cast<uint8_t>(multisampleFlags)) *
			std::popcount(static_cast<uint8_t>(depthFuncFlags)) *
			std::popcount(static_cast<uint8_t>(depthTestFlags)) *
			std::popcount(static_cast<uint8_t>(depthWriteFlags));
	}
	bool PipelineVariants::VariantExists(FillMode fillMode, CullMode cullMode, Multisamples multisamples, DepthFunc depthFunc, DepthTest depthTest, DepthWrite depthWrite) const
	{
		return
			static_cast<uint8_t>(fillModeFlags & fillMode) &&
			static_cast<uint8_t>(cullModeFlags & cullMode) &&
			static_cast<uint8_t>(multisampleFlags & multisamples) &&
			static_cast<uint8_t>(depthFuncFlags & depthFunc) &&
			static_cast<uint8_t>(depthTestFlags & depthTest) &&
			static_cast<uint8_t>(depthWriteFlags & depthWrite);
	}
	uint32_t PipelineVariants::GetVariantIndex(FillMode fillMode, CullMode cullMode, Multisamples multisamples, DepthFunc depthFunc, DepthTest depthTest, DepthWrite depthWrite) const
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

		// MUST BE DONE IN REVERSE ORDER
		addFlagToIndex(depthWrite, depthWriteFlags);
		addFlagToIndex(depthTest, depthTestFlags);
		addFlagToIndex(depthFunc, depthFuncFlags);
		addFlagToIndex(multisamples, multisampleFlags);
		addFlagToIndex(cullMode, cullModeFlags);
		addFlagToIndex(fillMode, fillModeFlags);

		return index;
	}
	PipelineVariants PipelineVariants::GetVariant(uint32_t index) const
	{
		PipelineVariants variant {};

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
		uint8_t depthWriteFlag = getNthSetBit<uint8_t>(static_cast<uint8_t>(depthWriteFlags), getFlagFromIndex(depthWriteFlags));
		uint8_t depthTestFlag = getNthSetBit<uint8_t>(static_cast<uint8_t>(depthTestFlags), getFlagFromIndex(depthTestFlags));
		uint8_t depthFuncFlag = getNthSetBit<uint8_t>(static_cast<uint8_t>(depthFuncFlags), getFlagFromIndex(depthFuncFlags));
		uint8_t multisampleFlag = getNthSetBit<uint8_t>(static_cast<uint8_t>(multisampleFlags), getFlagFromIndex(multisampleFlags));
		uint8_t cullModeFlag = getNthSetBit<uint8_t>(static_cast<uint8_t>(cullModeFlags), getFlagFromIndex(cullModeFlags));
		uint8_t fillModeFlag = getNthSetBit<uint8_t>(static_cast<uint8_t>(fillModeFlags), getFlagFromIndex(fillModeFlags));

		// Note that this function relies on the side effect from getNthSetBit where if the index is invalid, it returns 0;
		variant.depthWriteFlags = (depthWriteFlag) ? static_cast<DepthWrite>(depthWriteFlag) : DepthWrite::Disabled;
		variant.depthTestFlags = (depthTestFlag) ? static_cast<DepthTest>(depthTestFlag) : DepthTest::Disabled;
		variant.depthFuncFlags = (depthFuncFlag) ? static_cast<DepthFunc>(depthFuncFlag) : DepthFunc::Always;
		variant.multisampleFlags = (multisampleFlag) ? static_cast<Multisamples>(multisampleFlag) : Multisamples::SixtyFour;
		variant.cullModeFlags = (cullModeFlag) ? static_cast<CullMode>(cullModeFlag) : CullMode::None;
		variant.fillModeFlags = (fillModeFlag) ? static_cast<FillMode>(fillModeFlag) : FillMode::Point;

		return variant;
	}
	VkPolygonMode PipelineVariants::GetPolygonMode(FillMode fillMode)
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
	VkCullModeFlags PipelineVariants::GetCullMode(CullMode cullMode)
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
	VkSampleCountFlagBits PipelineVariants::GetMultisamples(Multisamples samples)
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
	VkCompareOp PipelineVariants::GetDepthFunc(DepthFunc depthFunc)
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
	VkBool32 PipelineVariants::GetDepthTest(DepthTest depthTest)
	{
		switch (depthTest)
		{
		case DepthTest::Enabled: return VK_TRUE;
		case DepthTest::Disabled: return VK_FALSE;
		}
		// TODO assert or quietly assert
		return VK_FALSE;
	}
	VkBool32 PipelineVariants::GetDepthWrite(DepthWrite depthWrite)
	{
		switch (depthWrite)
		{
		case DepthWrite::Enabled: return VK_TRUE;
		case DepthWrite::Disabled: return VK_FALSE;
		}
		// TODO assert or quietly assert
		return VK_FALSE;
	}
	PipelineVariants::Multisamples PipelineVariants::ConvertSamplesToVariant(uint32_t samples)
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
	// Generate optimised pipelines for specific use cases
	// Generates pipelines for double-sided and transparent objects
	PipelineVariants PipelineVariants::GetDefaultVariant(uint32_t samples)
	{
		return PipelineVariants(FillMode::Solid, CullMode::Back | CullMode::None, ConvertSamplesToVariant(samples), DepthFunc::GreaterEqual, DepthTest::Enabled, DepthWrite::Disabled);
	}
	PipelineVariants PipelineVariants::GetSkyboxVariant(uint32_t samples)
	{
		return PipelineVariants(FillMode::Solid, CullMode::Back, ConvertSamplesToVariant(samples), DepthFunc::Always, DepthTest::Enabled, DepthWrite::Disabled);
	}
	PipelineVariants PipelineVariants::GetShadowVariant(uint32_t samples)
	{
		return PipelineVariants(FillMode::Solid, CullMode::Front, ConvertSamplesToVariant(samples), DepthFunc::LessEqual, DepthTest::Enabled, DepthWrite::Enabled);
	}
	PipelineVariants PipelineVariants::GetDepthPrepassVariant(uint32_t samples)
	{
		return PipelineVariants(FillMode::Solid, CullMode::Back | CullMode::None, ConvertSamplesToVariant(samples), DepthFunc::GreaterEqual, DepthTest::Enabled, DepthWrite::Enabled);
	}
	PipelineVariants PipelineVariants::GetUIVariant(uint32_t samples)
	{
		return PipelineVariants(FillMode::Solid, CullMode::None, ConvertSamplesToVariant(samples), DepthFunc::Never, DepthTest::Disabled, DepthWrite::Disabled);
	}
	PipelineVariants PipelineVariants::GetPostProcessingVariant(uint32_t samples)
	{
		return PipelineVariants(FillMode::Solid, CullMode::None, ConvertSamplesToVariant(samples), DepthFunc::Always, DepthTest::Disabled, DepthWrite::Disabled);
	}
	PipelineVariants PipelineVariants::operator[](uint32_t index) const { return GetVariant(index); }
};
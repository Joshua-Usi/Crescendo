#pragma once

#include "common.hpp"
#include "vulkan/vulkan.h"

CS_NAMESPACE_BEGIN::Vulkan::Vk
{
	class PipelineVariants
	{
	public:
		constexpr static uint8_t FillModeCount = 3, CullModeCount = 3, DepthFuncCount = 7, DepthWriteCount = 2, DepthTestCount = 2;
		enum class FillMode : uint8_t { Solid = 0b1, Wireframe = 0b10, Point = 0b100 };
		enum class CullMode : uint8_t { Back = 0b1, Front = 0b10, None = 0b100 };
		enum class Multisamples : uint8_t { One = 0b1, Two = 0b10, Four = 0b100, Eight = 0b1000, Sixteen = 0b10000, ThirtyTwo = 0b100000, SixtyFour = 0b1000000 };
		enum class DepthFunc : uint8_t { Never = 0b1, Less = 0b10, Equal = 0b100, LessEqual = 0b1000, Greater = 0b10000, GreaterEqual = 0b100000, Always = 0b1000000 };
		enum class DepthTest : uint8_t { Enabled = 0b1, Disabled = 0b10 };
		enum class DepthWrite : uint8_t { Enabled = 0b1, Disabled = 0b10 };
	public:
		// Actual size is 6 bytes so we can have 2 more flags
		// Pad to 8 bytes for nicer alignment
		union {
			struct {
				FillMode fillModeFlags;
				CullMode cullModeFlags;
				Multisamples multisampleFlags;
				DepthFunc depthFuncFlags;
				DepthTest depthTestFlags;
				DepthWrite depthWriteFlags;
			};
			uint64_t flags;
		};
	public:
		PipelineVariants();
		PipelineVariants(FillMode fillMode, CullMode cullMode, Multisamples multisamples, DepthFunc depthFunc, DepthTest depthTest, DepthWrite depthWrite);
		PipelineVariants(const PipelineVariants&) = default;
		PipelineVariants(PipelineVariants&&) = default;
		PipelineVariants& operator=(const PipelineVariants&) = default;
		PipelineVariants& operator=(PipelineVariants&&) = default;
	public:
		uint32_t GetVariantCount() const;
		bool VariantExists(FillMode fillMode, CullMode cullMode, Multisamples multisamples, DepthFunc depthFunc, DepthTest depthTest, DepthWrite depthWrite) const;
		uint32_t GetVariantIndex(FillMode fillMode, CullMode cullMode, Multisamples multisamples, DepthFunc depthFunc, DepthTest depthTest, DepthWrite depthWrite) const;
		PipelineVariants GetVariant(uint32_t index) const;
	public:
		static VkPolygonMode GetPolygonMode(FillMode fillMode);
		static VkCullModeFlags GetCullMode(CullMode cullMode);
		static VkSampleCountFlagBits GetMultisamples(Multisamples samples);
		static VkCompareOp GetDepthFunc(DepthFunc depthFunc);
		static VkBool32 GetDepthTest(DepthTest depthTest);
		static VkBool32 GetDepthWrite(DepthWrite depthWrite);
		static Multisamples ConvertSamplesToVariant(uint32_t samples);
	public:
		// Generate optimised pipelines for specific use cases
		static PipelineVariants GetDefaultVariant(uint32_t samples = 1);
		static PipelineVariants GetSkyboxVariant(uint32_t samples = 1);
		static PipelineVariants GetShadowVariant(uint32_t samples = 1);
		static PipelineVariants GetDepthPrepassVariant(uint32_t samples = 1);
		static PipelineVariants GetUIVariant(uint32_t samples = 1);
		static PipelineVariants GetPostProcessingVariant(uint32_t samples = 1);
	public:
		PipelineVariants operator[](uint32_t index) const;
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

	CS_DEFINE_ENUM_CLASS_BITWISE_OPERATORS(PipelineVariants::FillMode);
	CS_DEFINE_ENUM_CLASS_BITWISE_OPERATORS(PipelineVariants::CullMode);
	CS_DEFINE_ENUM_CLASS_BITWISE_OPERATORS(PipelineVariants::Multisamples);
	CS_DEFINE_ENUM_CLASS_BITWISE_OPERATORS(PipelineVariants::DepthFunc);
	CS_DEFINE_ENUM_CLASS_BITWISE_OPERATORS(PipelineVariants::DepthTest);
	CS_DEFINE_ENUM_CLASS_BITWISE_OPERATORS(PipelineVariants::DepthWrite);
}
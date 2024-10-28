#pragma once
#include "common.hpp"
#include "Rendering/ResourceHandles.hpp"
#include "utils/Color.hpp"
#include "Rendering/PushConstantBuffer.hpp"
#include <variant>

CS_NAMESPACE_BEGIN
{
	struct Material : public Component
	{
		// Metallic-roughness pbr
		std::variant<Vulkan::TextureHandle, Color> albedo; // rgba, default to Color(255, 255, 255, 255)
		std::variant<Vulkan::TextureHandle, float> metallic; // r, default to 0.0f
		std::variant<Vulkan::TextureHandle, float> roughness; // r, default to 0.5f
		std::variant<Vulkan::TextureHandle, std::monostate> normal; // rgb, default to none
		std::variant<Vulkan::TextureHandle, float> ao; // r, default to 1.0f
		std::variant<Vulkan::TextureHandle, cs_std::math::vec3> emissive; // rgb, default to vec3(0.0f, 0.0f, 0.0f)

		// Principled BSDF
		// std::variant<Vulkan::TextureHandle, float> clearCoat; // r, default to 0.0f
		// std::variant<Vulkan::TextureHandle, float> clearCoatRoughness; // r, default to 0.0f
		// std::variant<Vulkan::TextureHandle, float> transmission; // r, default to 0.0f
		// std::variant<Vulkan::TextureHandle, float> subsurface; // r, default to 0.0f
		// std::variant<Vulkan::TextureHandle, float> sheen; // r default to 0.0f

		bool isTransparent;
		bool isDoubleSided;
		//bool isUnlit;
		bool isReceivingShadows;
		bool isShadowCasting;

		Material()
			: albedo(Color(255, 255, 255, 255)), metallic(0.0f), roughness(0.5f), normal(std::monostate()), ao(1.0f), emissive(cs_std::math::vec3(0.0f)), /*clearCoat(), clearCoatRoughness(), transmission(), subsurface(), sheen(),*/
			isTransparent(false), isDoubleSided(false), isReceivingShadows(true), isShadowCasting(true) {}
		Material(Vulkan::TextureHandle albedo, Vulkan::TextureHandle normal, bool isTransparent, bool isDoubleSided, bool isShadowCasting)
			: albedo(albedo), metallic(0.0f), roughness(0.5f), normal(normal), ao(1.0f), emissive(cs_std::math::vec3(0.0f)),
			isTransparent(isTransparent), isDoubleSided(isDoubleSided), isReceivingShadows(true), isShadowCasting(isShadowCasting) {}
	

		void BuildPushConstantBuffer(PushConstantBuffer<128>& buffer)
		{
			if (std::holds_alternative<Vulkan::TextureHandle>(albedo))
				buffer.Push(std::get<Vulkan::TextureHandle>(albedo).GetIndex());
			else
				buffer.Push(std::get<Color>(albedo));

			if (std::holds_alternative<Vulkan::TextureHandle>(metallic))
				buffer.Push(std::get<Vulkan::TextureHandle>(metallic).GetIndex());
			else
				buffer.Push(std::get<float>(metallic));

			if (std::holds_alternative<Vulkan::TextureHandle>(roughness))
				buffer.Push(std::get<Vulkan::TextureHandle>(roughness).GetIndex());
			else
				buffer.Push(std::get<float>(roughness));

			if (std::holds_alternative<Vulkan::TextureHandle>(normal))
				buffer.Push(std::get<Vulkan::TextureHandle>(normal).GetIndex());

			if (std::holds_alternative<Vulkan::TextureHandle>(ao))
				buffer.Push(std::get<Vulkan::TextureHandle>(ao).GetIndex());
			else
				buffer.Push(std::get<float>(ao));

			if (std::holds_alternative<Vulkan::TextureHandle>(emissive))
				buffer.Push(std::get<Vulkan::TextureHandle>(emissive).GetIndex());
			else
				buffer.Push(cs_std::math::vec4(std::get<cs_std::math::vec3>(emissive), 1.0f));
		}
		uint32_t GetPipelineIndex()
		{
			uint32_t index = 0;
			if (std::holds_alternative<Vulkan::TextureHandle>(emissive))
				index += 1;
			index <<= 1;
			if (std::holds_alternative<Vulkan::TextureHandle>(ao))
				index += 1;
			index <<= 1;
			if (std::holds_alternative<Vulkan::TextureHandle>(normal))
				index += 1;
			index <<= 1;
			if (std::holds_alternative<Vulkan::TextureHandle>(roughness))
				index += 1;
			index <<= 1;
			if (std::holds_alternative<Vulkan::TextureHandle>(metallic))
				index += 1;
			index <<= 1;
			if (std::holds_alternative<Vulkan::TextureHandle>(albedo))
				index += 1;
			return index;
		}
	};
}
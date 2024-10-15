#pragma once
#include "common.hpp"
#include "Rendering/ResourceHandles.hpp"
#include "utils/Color.hpp"

#include <variant>

CS_NAMESPACE_BEGIN
{
	struct Material : public Component
	{
		Vulkan::PipelineHandle pipeline;

		// Used for textured materials
		//Vulkan::TextureHandle diffuseHandle; // rgba
		//Vulkan::TextureHandle metallicHandle; // r
		//Vulkan::TextureHandle roughnessHandle; // r
		//Vulkan::TextureHandle normalHandle; // rgb
		//Vulkan::TextureHandle aoHandle; // r
		//Vulkan::TextureHandle emissiveHandle; // rgb
		//Vulkan::TextureHandle clearCoatHandle; // r
		//Vulkan::TextureHandle clearCoatRoughnessHandle; // r
		//Vulkan::TextureHandle transmissionHandle; // r
		//Vulkan::TextureHandle subsurfaceHandle; // r
		//Vulkan::TextureHandle sheenHandle; // r
		//Vulkan::TextureHandle alphaMaskHandle; // r

		//// Non-textured values
		//Color baseColor = Color(255, 255, 255, 255);
		//float metallic = 0.0f;
		//float roughness = 0.5f;
		//cs_std::math::vec3 normal = cs_std::math::vec3(0.0f, 0.0f, 1.0f);
		//float ao = 1.0f;
		//cs_std::math::vec3 emissive = cs_std::math::vec3(0.0f, 0.0f, 0.0f);
		//float clearCoat = 0.0f;
		//float clearCoatRoughness = 0.0f;
		//float transmission = 0.0f;
		//float subsurface = 0.0f;
		//float sheen = 0.0f;
		//float alphaMask = 0.0f;

		std::variant<Vulkan::TextureHandle, Color> albedo; // rgba
		std::variant<Vulkan::TextureHandle, float> metallic; // r
		std::variant<Vulkan::TextureHandle, float> roughness; // r
		std::variant<Vulkan::TextureHandle, cs_std::math::vec3> normal; // rgb
		std::variant<Vulkan::TextureHandle, float> ao; // r
		std::variant<Vulkan::TextureHandle, cs_std::math::vec3> emissive; // rgb
		std::variant<Vulkan::TextureHandle, float> clearCoat; // r
		std::variant<Vulkan::TextureHandle, float> clearCoatRoughness; // r
		std::variant<Vulkan::TextureHandle, float> transmission; // r
		std::variant<Vulkan::TextureHandle, float> subsurface; // r
		std::variant<Vulkan::TextureHandle, float> sheen; // r

		bool isTransparent;
		bool isDoubleSided;
		bool isUnlit;
		bool isReceivingShadows;
		bool isShadowCasting;
		bool isWritingToDepth;
		bool isTestingDepth;

		Material()
			: pipeline()
			, albedo(), metallic(), roughness(), normal(), ao(), emissive(), clearCoat(), clearCoatRoughness(), transmission(), subsurface(), sheen(),
			isTransparent(false), isDoubleSided(false), isUnlit(false), isReceivingShadows(true), isShadowCasting(true), isWritingToDepth(true), isTestingDepth(true) {}
		Material(Vulkan::PipelineHandle pipeline, Vulkan::TextureHandle albedo, Vulkan::TextureHandle normal, bool isTransparent, bool isDoubleSided, bool isShadowCasting) : pipeline(pipeline), albedo(albedo), normal(normal), isTransparent(isTransparent), isDoubleSided(isDoubleSided), isShadowCasting(isShadowCasting) {}
	};
}
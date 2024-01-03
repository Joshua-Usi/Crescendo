#pragma once

#include <string>

#include "cs_std/math/math.hpp"
#include "cs_std/graphics/algorithms.hpp"

namespace CrescendoEngine
{
	struct Component {};

	template<typename T>
	concept ValidComponent = std::derived_from<T, Component>;

	struct Name : public Component
	{
		std::string name;
		Name(const std::string& name) : name(name) {}
		Name(const char* name) : name(name) {}
	};

	struct Transform : public Component
	{
		cs_std::math::mat4 transform;
		Transform() : transform(cs_std::math::mat4(1.0f)) {}
		Transform(const cs_std::math::mat4& transform) : transform(transform) {}
		operator cs_std::math::mat4& () { return this->transform; }
	};

	struct Material : public Component
	{
		uint32_t pipelineID, diffuseID, normalID;
		bool isTransparent, isDoubleSided, isShadowCasting;

		Material() : pipelineID(0), diffuseID(0), normalID(0), isTransparent(false), isDoubleSided(false), isShadowCasting(true) {}
		Material(uint32_t pipelineID, uint32_t diffuseID, uint32_t normalID, bool isTransparent, bool isDoubleSided, bool isShadowCasting) : pipelineID(pipelineID), diffuseID(diffuseID), normalID(normalID), isTransparent(isTransparent), isDoubleSided(isDoubleSided), isShadowCasting(isShadowCasting) {}
	};

	struct MeshData : public Component
	{
		cs_std::graphics::bounding_aabb bounds;
		uint32_t meshID;

		MeshData() : bounds(), meshID(0) {}
		MeshData(const cs_std::graphics::bounding_aabb& bounds, uint32_t meshID) : bounds(bounds), meshID(meshID) {}
	};

	struct Skybox : public Component
	{
		uint32_t meshID, textureID;

		Skybox() : meshID(std::numeric_limits<uint32_t>::max()), textureID(std::numeric_limits<uint32_t>::max()) {}
		Skybox(uint32_t meshID, uint32_t textureID) : meshID(meshID), textureID(textureID) {}
	};

	struct Behaviour : public Component
	{
		// Pre-render
		void OnUpdate(double time) {};
		// Post-render
		void OnLateUpdate(double time) {};
		void OnAttach() {};
		void OnDetach() {};
	};
}
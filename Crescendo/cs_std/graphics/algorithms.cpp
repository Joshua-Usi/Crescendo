#include "algorithms.hpp"

#include "mikktspace/mikktspace.h"

#include <glm/gtc/matrix_access.hpp>


namespace cs_std::graphics
{
	namespace mikktspace
	{
		constexpr int get_vertices_per_face(const SMikkTSpaceContext* pContext, const int iFace)
		{
			// We only support triangles
			return 3;
		}
		int get_face_count(const SMikkTSpaceContext* pContext) {
			const mesh* pMesh = static_cast<mesh*>(pContext->m_pUserData);
			return pMesh->indices.size() / 3;
		}
		void get_position(const SMikkTSpaceContext* pContext, float fvPosOut[], const int iFace, const int iVert) {
			const mesh* pMesh = static_cast<mesh*>(pContext->m_pUserData);
			const int index = pMesh->indices[iFace * 3 + iVert];
			memcpy(fvPosOut, &pMesh->get_attribute(Attribute::POSITION).data[index * 3], sizeof(float) * 3);
		}
		void get_normal(const SMikkTSpaceContext* pContext, float fvNormOut[], const int iFace, const int iVert) {
			const mesh* pMesh = static_cast<mesh*>(pContext->m_pUserData);
			const int index = pMesh->indices[iFace * 3 + iVert];
			memcpy(fvNormOut, &pMesh->get_attribute(Attribute::NORMAL).data[index * 3], sizeof(float) * 3);
		}
		void get_tex_coord(const SMikkTSpaceContext* pContext, float fvTexcOut[], const int iFace, const int iVert) {
			const mesh* pMesh = static_cast<mesh*>(pContext->m_pUserData);
			const int index = pMesh->indices[iFace * 3 + iVert];
			memcpy(fvTexcOut, &pMesh->get_attribute(Attribute::TEXCOORD_0).data[index * 2], sizeof(float) * 2);
		}
		void set_tangent_space_basic(const SMikkTSpaceContext* pContext, const float fvTangent[], const float fSign, const int iFace, const int iVert) {
			mesh* pMesh = static_cast<mesh*>(pContext->m_pUserData);
			const int index = pMesh->indices[iFace * 3 + iVert];

			std::vector<float>& tangents = pMesh->get_attribute(Attribute::TANGENT).data;

			tangents[index * 4 + 0] = fvTangent[0];
			tangents[index * 4 + 1] = fvTangent[1];
			tangents[index * 4 + 2] = fvTangent[2];
			tangents[index * 4 + 3] = fSign;
		}
	}
	bounding_aabb::bounding_aabb(const std::vector<float>& vertices)
	{
		if (vertices.size() == 0)
		{
			this->min = this->max = glm::vec3(0.0f);
			return;
		}

		this->min.x = this->min.y = this->min.z = std::numeric_limits<float>::max();
		this->max.x = this->max.y = this->max.z = std::numeric_limits<float>::lowest();

		for (size_t i = 0; i < vertices.size(); i += 3) {
			this->min.x = std::min(this->min.x, vertices[i]);
			this->min.y = std::min(this->min.y, vertices[i + 1]);
			this->min.z = std::min(this->min.z, vertices[i + 2]);

			this->max.x = std::max(this->max.x, vertices[i]);
			this->max.y = std::max(this->max.y, vertices[i + 1]);
			this->max.z = std::max(this->max.z, vertices[i + 2]);
		}
	}
	bounding_aabb& bounding_aabb::transform(const glm::mat4& transform)
	{
		const glm::vec3 transformedCorners[8]
		{
			transform * glm::vec4(min.x, min.y, min.z, 1.0f),
			transform * glm::vec4(min.x, min.y, max.z, 1.0f),
			transform * glm::vec4(min.x, max.y, min.z, 1.0f),
			transform * glm::vec4(min.x, max.y, max.z, 1.0f),
			transform * glm::vec4(max.x, min.y, min.z, 1.0f),
			transform * glm::vec4(max.x, min.y, max.z, 1.0f),
			transform * glm::vec4(max.x, max.y, min.z, 1.0f),
			transform * glm::vec4(max.x, max.y, max.z, 1.0f),
		};
		glm::vec3	newMin = transformedCorners[0],
					newMax = transformedCorners[0];

		for (uint8_t i = 1; i < 8; i++)
		{
			newMin = glm::min(newMin, transformedCorners[i]);
			newMax = glm::max(newMax, transformedCorners[i]);
		}

		this->min = newMin;
		this->max = newMax;

		return *this;
	}
	frustum::frustum(const glm::mat4& viewProjection)
	{
		const glm::vec4 row[4] =
		{
			glm::row(viewProjection, 0),
			glm::row(viewProjection, 1),
			glm::row(viewProjection, 2),
			glm::row(viewProjection, 3)
		};

		const glm::vec4 coefficient[6] =
		{
			row[3] - row[1],
			row[3] + row[1],
			row[3] - row[0],
			row[3] + row[0],
			row[3] - row[2],
			row[3] + row[2]
		};

		for (uint8_t i = 0; i < 6; i++)
		{
			const glm::vec4 norm = glm::normalize(coefficient[i]);
			this->planes[i].normal = glm::vec3(norm);
			this->planes[i].distance = norm.w;
		}
	}
	bool frustum::intersects(const bounding_aabb& volume) const
	{
		const glm::vec3& min = volume.min, max = volume.max;

		for (uint8_t i = 0; i < 6; i++)
		{
			const glm::vec3& normal = this->planes[i].normal;
			const float distance = this->planes[i].distance;

			glm::vec3 positiveVertex = min;
			if (normal.x >= 0.0f) positiveVertex.x = max.x;
			if (normal.y >= 0.0f) positiveVertex.y = max.y;
			if (normal.z >= 0.0f) positiveVertex.z = max.z;

			if (glm::dot(normal, positiveVertex) + distance < 0.0f) return false;
		}
		return true;
	}
	void generate_tangents(mesh& inputMesh)
	{
		inputMesh.get_attribute(Attribute::TANGENT).data.resize(inputMesh.get_attribute(Attribute::POSITION).data.size() / 3 * 4);

		SMikkTSpaceInterface interface;
		interface.m_getNumFaces = mikktspace::get_face_count;
		interface.m_getNumVerticesOfFace = mikktspace::get_vertices_per_face;
		interface.m_getPosition = mikktspace::get_position;
		interface.m_getNormal = mikktspace::get_normal;
		interface.m_getTexCoord = mikktspace::get_tex_coord;
		interface.m_setTSpaceBasic = mikktspace::set_tangent_space_basic;
		interface.m_setTSpace = nullptr;

		SMikkTSpaceContext context;
		context.m_pInterface = &interface;
		context.m_pUserData = &inputMesh;

		const bool result = genTangSpaceDefault(&context);

		// TODO we should do something if we error
	}
}
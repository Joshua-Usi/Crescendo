#pragma once

#include <vector>
#include <limits>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_access.hpp>

namespace Crescendo::Algorithms
{
	struct Frustum
	{
		struct Plane { glm::vec3 normal; float distance; };
		// top, bottom, right, left, far, near
		Plane planes[6];
	};

	struct BoundingAABB
	{
		glm::vec3 min, max;

		/// <summary>
		/// Return the center of the AABB.
		/// </summary>
		/// <returns>Center of the AABB</returns>
		inline glm::vec3 GetCenter() const { return (this->min + this->max) * 0.5f; }

		inline BoundingAABB& Transform(const glm::mat4& transform)
		{
			glm::vec3 transformedCorners[8]
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
			for (uint32_t i = 1; i < 8; i++)
			{
				newMin = glm::min(newMin, transformedCorners[i]);
				newMax = glm::max(newMax, transformedCorners[i]);
			}
			this->min = newMin;
			this->max = newMax;

			return *this;
		}
	};

	/// <summary>
	/// Given a vector of vertices, calculate the bounding AABB of the mesh.
	/// </summary>
	/// <param name="vertices">Vertice of the mesh in the form of { x, y, z, x, y, z, ... }</param>
	/// <returns>AABB of the mesh</returns>
	BoundingAABB CalculateMeshBoundingAABB(const std::vector<float>& vertices) {
		BoundingAABB bbox = {};
		if (vertices.size() == 0) return bbox;

		bbox.min.x = bbox.min.y = bbox.min.z = std::numeric_limits<float>::max();
		bbox.max.x = bbox.max.y = bbox.max.z = std::numeric_limits<float>::lowest();

		for (size_t i = 0; i < vertices.size(); i += 3) {
			bbox.min.x = std::min(bbox.min.x, vertices[i]);
			bbox.min.y = std::min(bbox.min.y, vertices[i + 1]);
			bbox.min.z = std::min(bbox.min.z, vertices[i + 2]);

			bbox.max.x = std::max(bbox.max.x, vertices[i]);
			bbox.max.y = std::max(bbox.max.y, vertices[i + 1]);
			bbox.max.z = std::max(bbox.max.z, vertices[i + 2]);
		}

		return bbox;
	}

	Frustum GetFrustum(const glm::mat4& viewProjection)
	{
		Frustum frustum = {};

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

		for (uint32_t i = 0; i < 6; i++)
		{
			glm::vec4 norm = glm::normalize(coefficient[i]);
			frustum.planes[i].normal = glm::vec3(norm);
			frustum.planes[i].distance = norm.w;
		}

		return frustum;
	}

	bool IsInFrustum(const Frustum& frustum, const BoundingAABB& volume)
	{
		const glm::vec3& min = volume.min, max = volume.max;

		for (uint32_t i = 0; i < 6; i++)
		{
			const glm::vec3& normal = frustum.planes[i].normal;
			const float distance = frustum.planes[i].distance;

			glm::vec3 positiveVertex = min;
			if (normal.x >= 0.0f) positiveVertex.x = max.x;
			if (normal.y >= 0.0f) positiveVertex.y = max.y;
			if (normal.z >= 0.0f) positiveVertex.z = max.z;

			if (glm::dot(normal, positiveVertex) + distance < 0.0f) return false;
		}
		return true;
	}
}
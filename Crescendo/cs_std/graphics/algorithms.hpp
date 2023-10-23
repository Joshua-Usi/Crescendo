#pragma once

#include "mesh.hpp"

#include <glm/vec3.hpp>
#include <glm/matrix.hpp>

#include <vector>

namespace cs_std::graphics
{

	/// <summary>
	/// Generate the tangents for a specific mesh
	/// Note that this method returns a new mesh due to the nature of mikktspace
	/// </summary>
	/// <param name="mesh"></param>
	/// <returns></returns>
	void generate_tangents(mesh& inputMesh);

	struct bounding_aabb
	{
		glm::vec3 min, max;

		bounding_aabb() = default;
		inline bounding_aabb(const glm::vec3& min, const glm::vec3& max) : min(min), max(max) {}
		/// <summary>
		/// Generate a bounding box based on a set of vertices
		/// </summary>
		/// <param name="vertices">Bounding box to generate from</param>
		bounding_aabb(const std::vector<float>& vertices);

		/// <summary>
		/// Transform the AABB by the given matrix.
		/// </summary>
		/// <param name="transform"></param>
		/// <returns></returns>
		bounding_aabb& transform(const glm::mat4& transform);

		/// <summary>
		/// Return the center of the AABB.
		/// </summary>
		/// <returns>Center of the AABB</returns>
		inline glm::vec3 center() const
		{
			return (this->min + this->max) * 0.5f;
		}
	};

	struct frustum
	{
		struct plane { glm::vec3 normal; float distance; };
		// top, bottom, right, left, far, near
		plane planes[6];

		frustum() = default;
		/// <summary>
		/// Generate a view frustum from a given viewProjection, works with perspective and orthographic
		/// </summary>
		/// <param name="viewProjection">viewProjection matrix</param>
		frustum(const glm::mat4& viewProjection);

		/// <summary>
		/// Determines if a given aabb volume is either fully inside or intersecting the frustum
		/// </summary>
		/// <param name="volume">Bounding volume to compar</param>
		/// <returns>False if not inside or touching frustum, true otherwise</returns>
		bool intersects(const bounding_aabb& volume) const;
	};
}
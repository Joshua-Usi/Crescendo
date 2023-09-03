#pragma once

#include <vector>
#include <limits>

#include "glm/vec3.hpp"

namespace Crescendo::Algorithms
{
	struct AABB
	{
        glm::vec3 min;
        glm::vec3 max;

        /// <summary>
        /// Return the center of the AABB.
        /// </summary>
        /// <returns>Center of the AABB</returns>
        inline glm::vec3 GetCenter() const { return (this->min + this->max) * 0.5f; }
	};

    /// <summary>
    /// Given a vector of vertices, calculate the bounding AABB of the mesh.
    /// </summary>
    /// <param name="vertices">Vertice of the mesh in the form of { x, y, z, x, y, z, ... }</param>
    /// <returns>AABB of the mesh</returns>
    AABB CalculateMeshBoundingAABB(const std::vector<float>& vertices) {
        AABB bbox = {};

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
}
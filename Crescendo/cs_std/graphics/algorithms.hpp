#pragma once

#include <glm/vec3.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_access.hpp>

#include <vector>

namespace cs_std::graphics
{
	struct bounding_aabb
	{
		glm::vec3 min, max;

		bounding_aabb() = default;
		inline bounding_aabb(const glm::vec3& min, const glm::vec3& max) : min(min), max(max) {}
		inline bounding_aabb(const std::vector<float>& vertices)
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

		/// <summary>
		/// Return the center of the AABB.
		/// </summary>
		/// <returns>Center of the AABB</returns>
		inline glm::vec3 center() const
		{
			return (this->min + this->max) * 0.5f;
		}

		/// <summary>
		/// Transform the AABB by the given matrix.
		/// </summary>
		/// <param name="transform"></param>
		/// <returns></returns>
		inline bounding_aabb& transform(const glm::mat4& transform)
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
	};

	struct frustum
	{
		struct plane { glm::vec3 normal; float distance; };
		// top, bottom, right, left, far, near
		plane planes[6];

		frustum() = default;
		frustum(const glm::mat4& viewProjection)
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

		bool intersects(const bounding_aabb& volume) const
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
	};
}
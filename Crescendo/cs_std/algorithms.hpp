#pragma once

#include <cmath>
#include <algorithm>
#include <limits>
#include <vector>

namespace cs_std
{
    /// <summary>
    /// Given 2 vectors, return a new vector with the contents of both vectors
    /// </summary>
    /// <typeparam name="T">Underlying type</typeparam>
    /// <param name="v1">Vector 1, it's content is first</param>
    /// <param name="v2">Vector 2, it's content is second</param>
    /// <returns></returns>
    template <typename T>
    std::vector<T> combine(const std::vector<T>& v1, const std::vector<T>& v2) {
        if (v1.empty() && v2.empty()) return {};

        std::vector<T> result;

        result.reserve(v1.size() + v2.size());

        result.insert(result.end(), v1.begin(), v1.end());
        result.insert(result.end(), v2.begin(), v2.end());

        return result;
    }
}
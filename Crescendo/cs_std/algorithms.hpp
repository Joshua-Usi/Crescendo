#pragma once

#include <cmath>
#include <algorithm>
#include <limits>
#include <vector>

namespace cs_std
{
    /// <summary>
    /// Concatenate any number of vectors into a new vector.
    /// </summary>
    /// <typeparam name="T">Underlying type</typeparam>
    /// <typeparam name="...Vectors">Variadic template for multiple vector arguments</typeparam>
    /// <param name="vectors">A variadic number of vectors to be combined</param>
    /// <returns>A new vector with the contents of all given vectors</returns>
    template <typename T, typename... Vectors>
    std::vector<T> combine(const std::vector<T>& first, const Vectors&... vectors) {
        std::vector<T> result;
        result.reserve((first.size() + ... + vectors.size()));

        result.insert(result.end(), first.begin(), first.end());
        (result.insert(result.end(), vectors.begin(), vectors.end()), ...);

        return result;
    }
}
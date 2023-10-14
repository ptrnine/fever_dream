#pragma once

#include <cmath>

#include <SFML/System/Vector2.hpp>


namespace core {
inline auto lerp(auto v0, auto v1, auto t) {
    return v0 * (1.f - t) + v1 * t;
}

inline auto inverse_lerp(auto x1, auto x2, auto value) {
    return (value - x1) / (x2 - x1);
}

inline float magnitude2(const sf::Vector2f& vec) {
    return vec.x * vec.x + vec.y * vec.y;
}

inline float magnitude(const sf::Vector2f& vec) {
    return std::sqrt(magnitude2(vec));
}

inline sf::Vector2f normalize(const sf::Vector2f& vec) {
    return vec / magnitude(vec);
}

namespace dependence {
    template <typename T>
    T linear(T value) {
        return value;
    }

    template <typename T>
    T quadratic(T value) {
        return value * value;
    }

    template <typename T>
    T square(T value) {
        return std::pow(value, T(0.5));
    }
} // namespace dependence
} // namespace core

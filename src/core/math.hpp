#pragma once

#include <cmath>

#include <SFML/System/Vector2.hpp>


namespace core {
inline auto lerp(auto v0, auto v1, auto t) {
    return v0 * (1.f - t) + v1 * t;
}

inline auto bezier3(auto p0, auto p1, auto p2, auto p3, auto t) {
    auto inv_t  = decltype(t)(1.0) - t;
    auto inv_t2 = inv_t * inv_t;
    auto inv_t3 = inv_t2 * inv_t;
    auto t2     = t * t;
    auto t3     = t2 * t;
    return inv_t3 * p0 + 3 * inv_t2 * t * p1 + 3 * inv_t * t2 * p2 + t3 * p3;
}

inline auto cubic_bezier(auto x1, auto x2, auto p1_x, auto p1_y, auto p2_x, auto p2_y, auto t) {
    using T = decltype(t);
    auto x = bezier3(T(0), p1_x, p2_x, T(1), t);
    auto y = bezier3(T(0), p1_y, p2_y, T(1), t);
    return lerp(x1, x2, y);
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

namespace interpolation
{
    struct lerp {
        inline auto operator()(auto v0, auto v1, auto t) const {
            return core::lerp(v0, v1, t);
        }
    };

    template <typename T>
    struct cubic_bezier {
        cubic_bezier(T p1_x, T p1_y, T p2_x, T p2_y): p1x(p1_x), p1y(p1_y), p2x(p2_x), p2y(p2_y) {}

        inline auto operator()(auto v0, auto v1, auto t) const {
            return core::cubic_bezier(v0, v1, p1x, p1y, p2x, p2y, t);
        }

        T p1x, p1y, p2x, p2y;
    };

    template <typename T = float>
    auto ease() {
        return cubic_bezier(T(0.25), T(0.1), T(0.25), T(1));
    }

    template <typename T = float>
    auto ease_in() {
        return cubic_bezier(T(0.42), T(0), T(1), T(1));
    }

    template <typename T = float>
    auto ease_out() {
        return cubic_bezier(T(0), T(0), T(0.58), T(1));
    }
} // namespace interpolation

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
    T square_root(T value) {
        return std::pow(value, T(0.5));
    }
} // namespace dependence
} // namespace core

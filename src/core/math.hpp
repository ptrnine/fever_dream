#pragma once
#include <array>
#include <cmath>

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
    return p0 * inv_t3 + p1 * inv_t2 * t * 3 + p2 * 3 * inv_t * t2 + p3 * t3;
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

template <typename T>
inline auto clamp(T value, T min, T max) {
    return value < min ? min : (value > max ? max : value);
}

namespace interpolation
{
    namespace params {
        template <typename T = float>
        inline constexpr std::array ease = {T(0.25), T(0.1), T(0.25), T(1)};
        template <typename T = float>
        inline constexpr std::array ease_in = {T(0.42), T(0), T(1), T(1)};
        template <typename T = float>
        inline constexpr std::array ease_out = {T(0), T(0), T(0.58), T(1)};
    }

    struct lerp {
        inline auto operator()(auto v0, auto v1, auto t) const {
            return core::lerp(v0, v1, t);
        }
    };

    template <typename T>
    struct cubic_bezier {
        cubic_bezier(T p1_x, T p1_y, T p2_x, T p2_y): p1x(p1_x), p1y(p1_y), p2x(p2_x), p2y(p2_y) {}
        cubic_bezier(const std::array<T, 4>& params): p1x(params[0]), p1y(params[1]), p2x(params[2]), p2y(params[3]) {}

        inline auto operator()(auto v0, auto v1, auto t) const {
            return core::cubic_bezier(v0, v1, p1x, p1y, p2x, p2y, t);
        }

        T p1x, p1y, p2x, p2y;
    };

    template <typename T = float>
    auto ease() {
        return cubic_bezier(params::ease<T>);
    }

    template <typename T = float>
    auto ease_in() {
        return cubic_bezier(params::ease_in<T>);
    }

    template <typename T = float>
    auto ease_out() {
        return cubic_bezier(params::ease_out<T>);
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

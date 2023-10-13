#pragma once

#include <SFML/System/Vector2.hpp>

namespace grx::dependence {
template <typename T>
T linear(T value) {
    return value;
}

template <typename T>
T quadratic(T value) {
    return value * value;
}
} // namespace grx::dependence

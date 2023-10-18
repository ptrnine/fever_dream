#pragma once
#include <nlohmann/json.hpp>

#include "core/vec.hpp"

namespace nlohmann
{
inline void to_json(nlohmann::json& j, const core::vec2f& vec) {
    j = nlohmann::json{vec.x(), vec.y()};
}

inline void from_json(const nlohmann::json& j, core::vec2f& vec) {
    vec.x(j[0].get<float>());
    vec.y(j[1].get<float>());
}
} // namespace nlohmann

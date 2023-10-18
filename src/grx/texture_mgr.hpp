#pragma once
#include <map>

#include <SFML/Graphics/Texture.hpp>

namespace grx
{
struct texture_def {
    std::string path;
    bool smooth = true;
    bool srgb = false;
    bool repeated = false;

    auto operator<=>(const texture_def&) const = default;
};

class texture_mgr {
public:
    sf::Texture& load(const texture_def& def) {
        auto [it, was_insert] = cache.emplace(def, sf::Texture{});
        if (was_insert) {
            auto& texture = it->second;
            texture.loadFromFile(def.path);
            texture.setSmooth(def.smooth);
            texture.setSrgb(def.srgb);
            texture.setRepeated(def.repeated);
        }
        return it->second;
    }

    sf::Texture& load(const std::string& path, bool smooth = true, bool srgb = false, bool repeated = false) {
        return load(texture_def{path, smooth, srgb, repeated});
    }

private:
    std::map<texture_def, sf::Texture> cache;
};
} // namespace grx

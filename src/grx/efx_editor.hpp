#pragma once
#include <fstream>
#include <iostream>

#include "efx.hpp"
#include "json.hpp"
#include "keyframe_animation.hpp"
#include "texture_mgr.hpp"
#include "types.hpp"
#include "scene.hpp"

namespace grx::efx_editor
{
namespace json = nlohmann;

class efx_builder_error : public std::runtime_error {
public:
    efx_builder_error(std::string msg): std::runtime_error(std::move(msg)) {}
};

class efx_builder {
public:
    using object_t = json::json::object_t;
    using array_t = json::json::array_t;

    efx_builder(texture_mgr& texture_manager, const std::string& effect_path): tx_mgr(&texture_manager) {
        std::ifstream ifs(effect_path);
        if (!ifs.is_open())
            throw efx_builder_error("Cannot open effect file '" + effect_path + "'");

        auto b   = std::istreambuf_iterator<char>(ifs);
        auto e   = std::istreambuf_iterator<char>();
        efx_json = json::json::parse(b, e);
    }

    struct build_result_t {
        std::string name;
        efx effect;
    };

    build_result_t build() const {
        return build(efx_json);
    }

private:
    template <typename T>
    anim_key<T> parse_key(const object_t& obj) const {
        auto time = obj.at("time").get<float>();
        auto value_j = obj.at("value");
        auto type_p  = obj.find("type");

        auto type = interpolation_not_set;
        if (type_p != obj.end()) {
            auto type_str = type_p->second.get<std::string>();
            if (type_str == "bezier")
                type = interpolation_t::bezier;
            else if (type_str == "hold")
                type = interpolation_t::hold;
            else if (type_str == "linear")
                type = interpolation_t::linear;
            else
                throw efx_builder_error("Invalid interpolation type '" + type_str + "'");
        }

        auto in_p = obj.find("in");
        auto out_p = obj.find("out");

        vec2f in, out;

        if (in_p != obj.end()) {
            if (type == interpolation_not_set)
                type = interpolation_t::bezier;
            in = in_p->second.get<vec2f>();
        }

        if (out_p != obj.end()) {
            if (type == interpolation_not_set)
                type = interpolation_t::bezier;
            out = out_p->second.get<vec2f>();
        }

        if (type == interpolation_not_set)
            type = interpolation_t::linear;

        return anim_key<T>{value_j.get<T>(), time, type, in, out};
    }

    template <typename T>
    struct keys_result_t {
        anim_key_sequence<T> keys;
        std::vector<efx::handler_t::index_t> affected_indices;
        bool apply_to_all = true;
    };

    template <typename T>
    keys_result_t<T> parse_keys(const object_t& obj) const {
        keys_result_t<T> result;

        for (auto&& key : obj.at("keys").get<array_t>())
            result.keys.push(parse_key<T>(key));

        if (auto apply_to_p = obj.find("apply_to"); apply_to_p != obj.end()) {
            if (apply_to_p->second.is_string() && apply_to_p->second.get<std::string>() == "all")
                result.apply_to_all = true;
            else if (apply_to_p->second.is_array()) {
                result.affected_indices = apply_to_p->second.get<std::vector<efx::handler_t::index_t>>();
                result.apply_to_all = false;
            }
        }

        return result;
    }

    bool parse_set_source_rect(const object_t& obj, auto& drawable) const {
        if (auto value = obj.find("source_rect"); value != obj.end()) {
            auto rect = value->second.get<std::array<int, 4>>();
            drawable.setTextureRect({rect[0], rect[1], rect[2], rect[3]});
            return true;
        }
        return false;
    }

    bool
    parse_set_texture(const object_t& obj, auto& drawable, const std::map<std::string, sf::Texture*>& textures) const {
        if (auto value = obj.find("texture"); value != obj.end()) {
            auto name      = value->second.get<std::string>();
            auto texture_p = textures.find(name);

            if (texture_p == textures.end())
                throw efx_builder_error("Cannot find texture '" + name + "'");

            if constexpr (std::is_same_v<std::decay_t<decltype(drawable)>, sf::Sprite>)
                drawable.setTexture(*texture_p->second);
            else
                drawable.setTexture(texture_p->second);

            return true;
        }
        return false;
    }

#define DEF_PARSE_SET(what, sfml_method, cast)                                                                         \
    bool parse_set_##what(const object_t& obj, auto& drawable) const {                                                 \
        if (auto value = obj.find(#what); value != obj.end()) {                                                        \
            drawable.sfml_method(cast);                                                                                \
            return true;                                                                                               \
        }                                                                                                              \
        return false;                                                                                                  \
    }

    DEF_PARSE_SET(color, setColor, rgba_t::from_str(value->second.get<std::string>()))
    DEF_PARSE_SET(fill_color, setFillColor, rgba_t::from_str(value->second.get<std::string>()))
    DEF_PARSE_SET(position, setPosition, value->second.get<vec2f>())
    DEF_PARSE_SET(size, setSize, value->second.get<vec2f>())
    DEF_PARSE_SET(origin, setOrigin, value->second.get<vec2f>())
    DEF_PARSE_SET(radius, setRadius, value->second.get<float>())
    DEF_PARSE_SET(rotation, setRotation, value->second.get<float>())
    DEF_PARSE_SET(point_count, setPointCount, value->second.get<unsigned>())

#undef DEF_PARSE_SET

    drawable_t parse_template(const object_t& obj, const std::map<std::string, sf::Texture*>& textures) const {
        auto e = obj.end();

        auto type = obj.at("type").get<std::string>();

        if (type == "sprite") {
            sf::Sprite drawable;
            parse_set_texture(obj, drawable, textures);
            parse_set_color(obj, drawable);
            parse_set_source_rect(obj, drawable);
            parse_set_position(obj, drawable);
            parse_set_origin(obj, drawable);
            parse_set_rotation(obj, drawable);
            return drawable;
        }
        else if (type == "circle") {
            sf::CircleShape drawable;
            parse_set_texture(obj, drawable, textures);
            parse_set_fill_color(obj, drawable);
            parse_set_position(obj, drawable);
            parse_set_origin(obj, drawable);
            parse_set_radius(obj, drawable);
            parse_set_rotation(obj, drawable);
            parse_set_point_count(obj, drawable);
            return drawable;
        }
        else if (type == "rect") {
            sf::RectangleShape drawable;
            parse_set_texture(obj, drawable, textures);
            parse_set_fill_color(obj, drawable);
            parse_set_position(obj, drawable);
            parse_set_size(obj, drawable);
            parse_set_origin(obj, drawable);
            parse_set_rotation(obj, drawable);
            return drawable;
        }
        else {
            throw efx_builder_error("Invalid template type: '" + type + "'");
        }
    }

    build_result_t build(const object_t& obj) const {
        build_result_t result;

        result.name = obj.at("name").get<std::string>();
        float duration = obj.at("duration").get<float>();

        /*
         * Parse animations
         */
        for (auto&& anim_j : obj.at("animations").get<array_t>()) {
            auto anim_obj = anim_j.get<object_t>();
            auto name = anim_obj.at("name").get<std::string>();
            auto type = anim_obj.at("type").get<std::string>();

            /* TODO: refactor */
            if (type == "position") {
                auto keys_result = parse_keys<vec2f>(anim_obj);
                auto& handler = result.effect.add_handler(name, efx_handlers::position(keys_result.keys));
                if (keys_result.apply_to_all)
                    handler.set_affects_all(true);
                else
                    handler.set_affected_indices(keys_result.affected_indices);
            }
            else if (type == "scale") {
                auto keys_result = parse_keys<vec2f>(anim_obj);
                auto& handler = result.effect.add_handler(name, efx_handlers::scale(keys_result.keys));
                if (keys_result.apply_to_all)
                    handler.set_affects_all(true);
                else
                    handler.set_affected_indices(keys_result.affected_indices);
            }
            else if (type == "rotation") {
                auto keys_result = parse_keys<float>(anim_obj);
                auto& handler = result.effect.add_handler(name, efx_handlers::rotation(keys_result.keys));
                if (keys_result.apply_to_all)
                    handler.set_affects_all(true);
                else
                    handler.set_affected_indices(keys_result.affected_indices);
            }
            else {
                throw efx_builder_error("Invalid animation type '" + type + "'");
            }
        }

        /*
         * Parse textures
         */
        std::map<std::string, sf::Texture*> textures;
        for (auto&& [texture_name, texture_j] : obj.at("textures").get<object_t>()) {
            auto tx_obj = texture_j.get<object_t>();
            auto path = tx_obj.at("path").get<std::string>();
            auto smooth_p = tx_obj.find("smooth");
            auto srgb_p = tx_obj.find("srgb");
            auto repeated_p = tx_obj.find("repeated");

            bool smooth = true;
            bool srgb = false;
            bool repeated = false;

            if (smooth_p != tx_obj.end())
                smooth = smooth_p->second.get<bool>();
            if (srgb_p != tx_obj.end())
                srgb = srgb_p->second.get<bool>();
            if (repeated_p != tx_obj.end())
                repeated = repeated_p->second.get<bool>();

            textures.emplace(texture_name, &tx_mgr->load(path, smooth, srgb, repeated));
        }

        /*
         * Parse templates
         */
        std::map<std::string, drawable_t> templates;
        for (auto&& [template_name, template_j] : obj.at("templates").get<object_t>())
            templates.emplace(template_name, parse_template(template_j.get<object_t>(), textures));

        /* Parse primitives */
        for (auto&& prim_j : obj.at("primitives").get<array_t>()) {
            auto prim_obj = prim_j.get<object_t>();

            auto template_name = prim_obj.at("template").get<std::string>();
            auto template_p = templates.find(template_name);
            if (template_p == templates.end())
                throw efx_builder_error("Cannot find template '" + template_name + "'");

            result.effect.push_element(template_p->second);
        }

        return result;
    }

private:
    texture_mgr* tx_mgr;
    json::json efx_json;
};
} // namespace grx::efx_editor

#pragma once

#include <variant>

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/ConvexShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/VertexBuffer.hpp>

namespace grx {
using drawable_t = std::variant<sf::Sprite, sf::Text, sf::CircleShape, sf::ConvexShape, sf::RectangleShape>;

template <class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};

template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

template <typename Drawable, typename... Ts>
bool downcast(Drawable&& obj, auto&& handler, Ts&&... args) {
    return std::visit(
        [&, ...args = std::forward<Ts>(args)]<typename T>(T&& obj) mutable {
            if constexpr (requires { handler(std::forward<T>(obj), std::forward<Ts>(args)...); }) {
                handler(std::forward<T>(obj), std::forward<Ts>(args)...);
                return true;
            }
            return false;
        },
        std::forward<Drawable>(obj));
}
} // namespace grx

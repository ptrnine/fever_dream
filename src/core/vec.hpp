#pragma once
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <tuple>
#include <string>

#include "floating_point.hpp"
#include "concepts.hpp"
#include "vec_macro_gen.hpp"

#include <SFML/System/Vector2.hpp>
#include <SFML/System/Vector3.hpp>
#include <SFML/Graphics/Color.hpp>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

namespace core
{
/*
 * vector operations
 */

template <typename A, typename B, size_t... Idxs>
constexpr inline bool vec_integer_equal(const std::array<A, sizeof...(Idxs)>& a,
                                        const std::array<B, sizeof...(Idxs)>& b,
                                        std::index_sequence<Idxs...>&&) {
    return true && ((std::get<Idxs>(a) == std::get<Idxs>(b)) && ...);
}

template <typename A, size_t... Idxs>
constexpr inline auto vec_float_essentially_equal(const std::array<A, sizeof...(Idxs)>& a,
                                                  const std::array<A, sizeof...(Idxs)>& b,
                                                  A                                     epsilon,
                                                  std::index_sequence<Idxs...>&&) {
    return true && (essentially_equal(std::get<Idxs>(a), std::get<Idxs>(b), epsilon) && ...);
}

template <typename A, size_t... Idxs>
constexpr inline auto vec_float_approx_equal(const std::array<A, sizeof...(Idxs)>& a,
                                             const std::array<A, sizeof...(Idxs)>& b,
                                             A                                     epsilon,
                                             std::index_sequence<Idxs...>&&) {
    return true && (approx_equal(std::get<Idxs>(a), std::get<Idxs>(b), epsilon) && ...);
}

#define VECTOR_GEN_OP(NAME, OPERATOR)                                                                                  \
    template <typename A, typename B, size_t... Idxs>                                                                  \
    constexpr inline auto vec_##NAME(const std::array<A, sizeof...(Idxs)>& a,                                          \
                                     const std::array<B, sizeof...(Idxs)>& b,                                          \
                                     std::index_sequence<Idxs...>&&) {                                                 \
        return std::array{(std::get<Idxs>(a) OPERATOR std::get<Idxs>(b))...};                                          \
    }

VECTOR_GEN_OP(add, +)
VECTOR_GEN_OP(sub, -)
VECTOR_GEN_OP(mul, *)
VECTOR_GEN_OP(div, /)
#undef VECTOR_GEN_OP

#define VECTOR_FETCH_GEN_OP(NAME, OPERATOR)                                                                            \
    template <typename A, typename B, size_t... Idxs>                                                                  \
    constexpr inline void vec_fetch_##NAME(                                                                            \
        std::array<A, sizeof...(Idxs)>& a, const std::array<B, sizeof...(Idxs)>& b, std::index_sequence<Idxs...>&&) {  \
        ((std::get<Idxs>(a) OPERATOR std::get<Idxs>(b)), ...);                                                         \
    }

VECTOR_FETCH_GEN_OP(add, +=)
VECTOR_FETCH_GEN_OP(sub, -=)
VECTOR_FETCH_GEN_OP(mul, *=)
VECTOR_FETCH_GEN_OP(div, /=)
#undef VECTOR_FETCH_GEN_OP

template <typename A, size_t... Idxs>
inline A vec_magnitude_2(const std::array<A, sizeof...(Idxs)>& a, std::index_sequence<Idxs...>&&) {
    return ((std::get<Idxs>(a) * std::get<Idxs>(a)) + ...);
}

template <typename A, size_t Size>
inline A vec_magnitude(const std::array<A, Size>& a) {
    if constexpr (sizeof(A) == 4)
        return sqrtf(vec_magnitude_2(a, std::make_index_sequence<Size>()));
    else
        return sqrt(vec_magnitude_2(a, std::make_index_sequence<Size>()));
}

template <typename A, size_t... Idxs>
inline std::array<A, sizeof...(Idxs)>
vec_normalize(const std::array<A, sizeof...(Idxs)>& a, std::index_sequence<Idxs...>&&) {
    auto magnitude = vec_magnitude(a);
    return std::array{(std::get<Idxs>(a) / magnitude)...};
}

template <typename A, size_t... Idxs>
inline void vec_fetch_normalize(std::array<A, sizeof...(Idxs)>& a, std::index_sequence<Idxs...>&&) {
    auto magnitude = vec_magnitude(a);
    ((std::get<Idxs>(a) /= magnitude), ...);
}

template <typename A, typename B, size_t... Idxs>
constexpr inline auto vec_dot_product(const std::array<A, sizeof...(Idxs)>& a,
                                      const std::array<B, sizeof...(Idxs)>& b,
                                      std::index_sequence<Idxs...>&&) {
    return ((std::get<Idxs>(a) * std::get<Idxs>(b)) + ...);
}

#define VECTOR_SCALAR_GEN_OP(NAME, OPERATOR)                                                                           \
    template <typename A, typename N, size_t... Idxs>                                                                  \
    constexpr inline auto vec_scalar_##NAME(                                                                           \
        const std::array<A, sizeof...(Idxs)>& a, N scalar, std::index_sequence<Idxs...>&&) {                           \
        return std::array{(std::get<Idxs>(a) OPERATOR scalar)...};                                                     \
    }

VECTOR_SCALAR_GEN_OP(add, +)
VECTOR_SCALAR_GEN_OP(sub, -)
VECTOR_SCALAR_GEN_OP(mul, *)
VECTOR_SCALAR_GEN_OP(div, /)
#undef VECTOR_SCALAR_GEN_OP

#define VECTOR_SCALAR_FETCH_GEN_OP(NAME, OPERATOR)                                                                     \
    template <typename A, typename N, size_t... Idxs>                                                                  \
    constexpr inline auto vec_scalar_fetch_##NAME(                                                                     \
        std::array<A, sizeof...(Idxs)>& a, N scalar, std::index_sequence<Idxs...>&&) {                                 \
        ((std::get<Idxs>(a) OPERATOR scalar), ...);                                                                    \
    }

VECTOR_SCALAR_FETCH_GEN_OP(add, +=)
VECTOR_SCALAR_FETCH_GEN_OP(sub, -=)
VECTOR_SCALAR_FETCH_GEN_OP(mul, *=)
VECTOR_SCALAR_FETCH_GEN_OP(div, /=)
#undef VECTOR_SCALAR_FETCH_GEN_OP

template <typename T1, typename T2, size_t... Idxs>
constexpr inline auto vec_static_cast(const std::array<T2, sizeof...(Idxs)>& a, std::index_sequence<Idxs...>&&) {
    return std::array{static_cast<T1>(std::get<Idxs>(a))...};
}

template <typename T, size_t... Idxs>
constexpr inline auto vec_unary_minus(const std::array<T, sizeof...(Idxs)>& a, std::index_sequence<Idxs...>&&) {
    return std::array{-std::get<Idxs>(a)...};
}

template <typename T, size_t... Idxs>
std::array<T, sizeof...(Idxs)> vec_filled_with(T v, std::index_sequence<Idxs...>&&) {
    return std::array<T, sizeof...(Idxs)>{(void(Idxs), v)...};
}

/*
 * Basic vector
 */

template <typename T, size_t S, template <typename, size_t> class DerivedT>
struct vec_base {
    using n_dimension_vector = void;
    using value_type         = T;

    static constexpr size_t size() noexcept {
        return S;
    }

    std::array<T, S> v;

    static constexpr DerivedT<T, S> filled_with(T value) {
        return DerivedT<T, S>{vec_filled_with(value, std::make_index_sequence<S>())};
    }

    template <typename TT>
        requires requires { typename TT::n_dimension_vector; }
    constexpr explicit operator TT() const {
        return TT{vec_static_cast<typename TT::value_type>(v, std::make_index_sequence<S>())};
    }

    template <size_t N>
    constexpr decltype(auto) get() const noexcept {
        return std::get<N>(v);
    }

    template <size_t N>
    constexpr decltype(auto) get() noexcept {
        return std::get<N>(v);
    }

    constexpr auto operator-() const {
        return DerivedT{vec_unary_minus(v, std::make_index_sequence<S>())};
    }

    template <typename TT>
    constexpr auto operator+(const vec_base<TT, S, DerivedT>& vec) const {
        return DerivedT{vec_add(v, vec.v, std::make_index_sequence<S>())};
    }

    template <typename TT>
    constexpr auto operator-(const vec_base<TT, S, DerivedT>& vec) const {
        return DerivedT{vec_sub(v, vec.v, std::make_index_sequence<S>())};
    }

    template <typename TT>
    constexpr auto operator*(const vec_base<TT, S, DerivedT>& vec) const {
        return DerivedT{vec_mul(v, vec.v, std::make_index_sequence<S>())};
    }

    template <typename TT>
    constexpr auto operator/(const vec_base<TT, S, DerivedT>& vec) const {
        return DerivedT{vec_div(v, vec.v, std::make_index_sequence<S>())};
    }

    template <typename TT>
    constexpr DerivedT<T, S>& operator+=(const vec_base<TT, S, DerivedT>& vec) {
        vec_fetch_add(v, vec.v, std::make_index_sequence<S>());
        return static_cast<DerivedT<T, S>&>(*this);
    }

    template <typename TT>
    constexpr DerivedT<T, S>& operator-=(const vec_base<TT, S, DerivedT>& vec) {
        vec_fetch_sub(v, vec.v, std::make_index_sequence<S>());
        return static_cast<DerivedT<T, S>&>(*this);
    }

    template <typename TT>
    constexpr DerivedT<T, S>& operator*=(const vec_base<TT, S, DerivedT>& vec) {
        vec_fetch_mul(v, vec.v, std::make_index_sequence<S>());
        return static_cast<DerivedT<T, S>&>(*this);
    }

    template <typename TT>
    constexpr DerivedT<T, S>& operator/=(const vec_base<TT, S, DerivedT>& vec) {
        vec_fetch_div(v, vec.v, std::make_index_sequence<S>());
        return static_cast<DerivedT<T, S>&>(*this);
    }

    constexpr auto operator+(int_or_float auto n) const {
        return DerivedT{vec_scalar_add(this->v, n, std::make_index_sequence<S>())};
    }

    constexpr auto operator-(int_or_float auto n) const {
        return DerivedT{vec_scalar_sub(this->v, n, std::make_index_sequence<S>())};
    }

    constexpr auto operator*(int_or_float auto n) const {
        return DerivedT{vec_scalar_mul(this->v, n, std::make_index_sequence<S>())};
    }

    constexpr auto operator/(int_or_float auto n) const {
        return DerivedT{vec_scalar_div(this->v, n, std::make_index_sequence<S>())};
    }

    constexpr DerivedT<T, S>& operator+=(int_or_float auto n) {
        vec_scalar_fetch_add(this->v, n, std::make_index_sequence<S>());
        return static_cast<DerivedT<T, S>&>(*this);
    }

    constexpr DerivedT<T, S>& operator-=(int_or_float auto n) {
        vec_scalar_fetch_sub(this->v, n, std::make_index_sequence<S>());
        return static_cast<DerivedT<T, S>&>(*this);
    }

    constexpr DerivedT<T, S>& operator*=(int_or_float auto n) {
        vec_scalar_fetch_mul(this->v, n, std::make_index_sequence<S>());
        return static_cast<DerivedT<T, S>&>(*this);
    }

    constexpr DerivedT<T, S>& operator/=(int_or_float auto n) {
        vec_scalar_fetch_div(this->v, n, std::make_index_sequence<S>());
        return static_cast<DerivedT<T, S>&>(*this);
    }

    constexpr T magnitude_2() const {
        return vec_magnitude_2(this->v, std::make_index_sequence<S>());
    }

    template <typename TT>
    constexpr auto dot(const vec_base<TT, S, DerivedT>& vec) const {
        return vec_dot_product(this->v, vec.v, std::make_index_sequence<S>());
    }
};

/*
 * Non-floating point specific
 */
template <typename T, size_t S, template <typename, size_t> class DerivedT>
struct vec_specific : vec_base<T, S, DerivedT> {};

/*
 * Uint-8 specific (colors)
 */
template <template <typename, size_t> class DerivedT>
struct vec_specific<uint8_t, 4, DerivedT> : vec_base<uint8_t, 4, DerivedT> {
    operator sf::Color() const {
        return {this->v[0], this->v[1], this->v[2], this->v[3]};
    }
};

/*
 * Floating point specific
 */
template <std::floating_point T, size_t S, template <typename, size_t> class DerivedT>
struct vec_specific<T, S, DerivedT> : vec_base<T, S, DerivedT> {
    bool essentially_equal(const DerivedT<T, S>& vec, T epsilon) const {
        return vec_float_essentially_equal(this->v, vec.v, epsilon, std::make_index_sequence<S>());
    }

    bool approx_equal(const DerivedT<T, S>& vec, T epsilon) const {
        return vec_float_approx_equal(this->v, vec.v, epsilon, std::make_index_sequence<S>());
    }

    bool binary_equal(const DerivedT<T, S>& vec) const {
        return ::memcmp(this->v.data(), vec.v.data(), sizeof(T) * S) == 0;
    }

    T magnitude() const {
        return vec_magnitude(this->v);
    }

    DerivedT<T, S> normalize() const {
        return DerivedT<T, S>{vec_normalize(this->v, std::make_index_sequence<S>())};
    }

    DerivedT<T, S>& make_normalize() {
        vec_fetch_normalize(this->v, std::make_index_sequence<S>());
        return static_cast<DerivedT<T, S>&>(*this);
    }
};

/*
 * Vectors by dimmensions
 */

template <typename T, size_t S, template <typename, size_t> class DerivedT>
struct vec1_base : public vec_specific<T, S, DerivedT> {
    void x(T _x) {
        this->template get<0>() = _x;
    }
    T x() const {
        return this->template get<0>();
    }
    T& x() {
        return this->template get<0>();
    }

    void r(T _r) {
        this->x(_r);
    }
    T r() const {
        return this->x();
    }
    T& r() {
        return this->x();
    }
};

template <typename T, size_t S, template <typename, size_t> class DerivedT>
struct vec2_base : public vec1_base<T, S, DerivedT> {
    void y(T _y) {
        this->template get<1>() = _y;
    }
    T y() const {
        return this->template get<1>();
    }
    T& y() {
        return this->template get<1>();
    }

    void g(T _g) {
        this->y(_g);
    }
    T g() const {
        return this->y();
    }
    T& g() {
        return this->y();
    }

    operator sf::Vector2<T>() const {
        return {this->x(), this->y()};
    }

    operator ImVec2() const {
        return {this->x(), this->y()};
    }

    GEN_GET_2_FROM_VEC2(x, y)
    GEN_SET_2_FROM_VEC2(x, y)
    GEN_GET_2_FROM_VEC2(r, g)
    GEN_SET_2_FROM_VEC2(r, g)
};

template <typename T, size_t S, template <typename, size_t> class DerivedT>
struct vec3_base : public vec2_base<T, S, DerivedT> {
    void z(T _z) {
        this->template get<2>() = _z;
    }
    T z() const {
        return this->template get<2>();
    }
    T& z() {
        return this->template get<2>();
    }

    void b(T _b) {
        this->z(_b);
    }
    T b() const {
        return this->z();
    }
    T& b() {
        return this->z();
    }

    operator sf::Vector3<T>() const {
        return {this->x(), this->y(), z()};
    }

    GEN_GET_2_FROM_VEC3(x, y, z)
    GEN_GET_2_FROM_VEC3(r, g, b)
    GEN_GET_3_FROM_VEC3(x, y, z)
    GEN_GET_3_FROM_VEC3(r, g, b)

    GEN_SET_2_FROM_VEC3(x, y, z)
    GEN_SET_2_FROM_VEC3(r, g, b)
    GEN_SET_3_FROM_VEC3(x, y, z)
    GEN_SET_3_FROM_VEC3(r, g, b)
};

template <typename T, size_t S, template <typename, size_t> class DerivedT>
struct vec4_base : public vec3_base<T, S, DerivedT> {
    void w(T _w) {
        this->template get<3>() = _w;
    }
    T w() const {
        return this->template get<3>();
    }
    T& w() {
        return this->template get<3>();
    }

    void a(T _a) {
        this->w(_a);
    }
    T a() const {
        return this->w();
    }
    T& a() {
        return this->w();
    }

    GEN_GET_2_FROM_VEC4(x, y, z, w)
    GEN_GET_2_FROM_VEC4(r, g, b, a)
    GEN_GET_3_FROM_VEC4(x, y, z, w)
    GEN_GET_3_FROM_VEC4(r, g, b, a)
    GEN_GET_4_FROM_VEC4(x, y, z, w)
    GEN_GET_4_FROM_VEC4(r, g, b, a)

    GEN_SET_2_FROM_VEC4(x, y, z, w)
    GEN_SET_2_FROM_VEC4(r, g, b, a)
    GEN_SET_3_FROM_VEC4(x, y, z, w)
    GEN_SET_3_FROM_VEC4(r, g, b, a)
    GEN_SET_4_FROM_VEC4(x, y, z, w)
    GEN_SET_4_FROM_VEC4(r, g, b, a)
};

/*
 * Universal vector
 */

template <typename T, size_t S>
struct vec : public vec_specific<T, S, vec> {};

template <typename T>
struct vec<T, 1> : public vec1_base<T, 1, vec> {
    void set(T x) {
        this->x(x);
    }
};

#define DEF_SFML_OP(SFML_TYPE, OP, OP2, S1, S2)                                                                        \
    template <typename TT>                                                                                             \
    friend vec OP(const vec& v, const SFML_TYPE<TT>& sfml_vec) {                                                       \
        return v S1 vec(sfml_vec);                                                                                     \
    }                                                                                                                  \
                                                                                                                       \
    template <typename TT>                                                                                             \
    friend vec OP(const SFML_TYPE<T>& sfml_vec, const vec& v) {                                                        \
        return vec(v S1 sfml_vec);                                                                                     \
    }                                                                                                                  \
                                                                                                                       \
    template <typename TT>                                                                                             \
    friend vec& OP2(vec& v, const SFML_TYPE<TT>& sfml_vec) {                                                            \
        return v S2 vec(sfml_vec);                                                                                     \
    }

template <typename T>
struct vec<T, 2> : public vec2_base<T, 2, vec> {
    constexpr vec() = default;
    explicit constexpr vec(const std::array<T, 2>& a): vec2_base<T, 2, vec>{a} {}
    constexpr vec(T x, T y): vec2_base<T, 2, vec>{x, y} {}
    explicit constexpr vec(const sf::Vector2<T>& sfml_vec): vec2_base<T, 2, vec>{sfml_vec.x, sfml_vec.y} {}
    constexpr vec(const ImVec2& imgui_vec): vec2_base<T, 2, vec>{imgui_vec.x, imgui_vec.y} {}

    constexpr vec& operator=(const sf::Vector2<T>& sfml_vec) {
        this->xy(sfml_vec.x, sfml_vec.y);
    }

    void set(T x, T y) {
        this->xy(x, y);
    }

    DEF_SFML_OP(sf::Vector2, operator+, operator+=, +, +=)
    DEF_SFML_OP(sf::Vector2, operator-, operator-=, -, -=)
    DEF_SFML_OP(sf::Vector2, operator*, operator*=, *, *=)
    DEF_SFML_OP(sf::Vector2, operator/, operator/=, /, /=)
};

template <typename T>
struct vec<T, 3> : public vec3_base<T, 3, vec> {
    constexpr vec() = default;
    explicit constexpr vec(const std::array<T, 3>& a): vec3_base<T, 3, vec>{a} {}
    constexpr vec(T x, T y, T z): vec3_base<T, 3, vec>{x, y, z} {}
    constexpr vec(const vec<T, 2>& v, T z): vec{v.x(), v.y(), z} {}
    constexpr vec(T x, const vec<T, 2>& v): vec{x, v.x(), v.y()} {}
    explicit constexpr vec(const sf::Vector3<T>& sfml_vec): vec2_base<T, 3, vec>{sfml_vec.x, sfml_vec.y, sfml_vec.z} {}
    constexpr vec(T x, const sf::Vector2<T>& sfml_vec): vec2_base<T, 3, vec>{x, sfml_vec.x, sfml_vec.y} {}
    constexpr vec(const sf::Vector2<T>& sfml_vec, T z): vec2_base<T, 3, vec>{sfml_vec.x, sfml_vec.y, z} {}

    constexpr vec& operator=(const sf::Vector3<T>& sfml_vec) {
        this->set(sfml_vec.x, sfml_vec.y, sfml_vec.z);
    }

    void set(T x, T y, T z) {
        this->xyz(x, y, z);
    }

    template <typename TT>
    vec<T, 3> cross(const vec<TT, 3>& _vec) {
        return vec{this->y() * _vec.z() - this->z() * _vec.y(),
                   this->z() * _vec.x() - this->x() * _vec.z(),
                   this->x() * _vec.y() - this->y() * _vec.x()};
    }

    DEF_SFML_OP(sf::Vector3, operator+, operator+=, +, +=)
    DEF_SFML_OP(sf::Vector3, operator-, operator-=, -, -=)
    DEF_SFML_OP(sf::Vector3, operator*, operator*=, *, *=)
    DEF_SFML_OP(sf::Vector3, operator/, operator/=, /, /=)
};

#undef DEF_SFML_OP

template <typename T>
struct vec<T, 4> : public vec4_base<T, 4, vec> {
    constexpr vec() = default;
    explicit constexpr vec(const std::array<T, 4>& a): vec4_base<T, 4, vec>{a} {}
    constexpr vec(T x, T y, T z, T w): vec4_base<T, 4, vec>{x, y, z, w} {}
    constexpr vec(const vec<T, 2>& v, T z, T w): vec{v.x(), v.y(), z, w} {}
    constexpr vec(T x, const vec<T, 2>& v, T w): vec{x, v.x(), v.y(), w} {}
    constexpr vec(T x, T y, const vec<T, 2>& v): vec{x, y, v.x(), v.y()} {}
    constexpr vec(const vec<T, 3>& v, T w): vec{v.x(), v.y(), v.z(), w} {}
    constexpr vec(T x, const vec<T, 3>& v): vec{x, v.x(), v.y(), v.z()} {}

    void set(T x, T y, T z, T w) {
        this->xyzw(x, y, z, w);
    }
};

template <>
struct vec<uint8_t, 4> : public vec4_base<uint8_t, 4, vec> {
    using T         = uint8_t;
    constexpr vec() = default;
    explicit constexpr vec(const std::array<T, 4>& array): vec4_base<T, 4, vec>{array} {}
    constexpr vec(T r, T g, T b, T a): vec4_base<T, 4, vec>{r, g, b, a} {}
    constexpr vec(const vec<T, 2>& rg, T b, T a): vec{rg.r(), rg.g(), b, a} {}
    constexpr vec(T r, const vec<T, 2>& gb, T a): vec{r, gb.x(), gb.y(), a} {}
    constexpr vec(T r, T g, const vec<T, 2>& ba): vec{r, g, ba.x(), ba.y()} {}
    constexpr vec(const vec<T, 3>& rgb, T a): vec{rgb.r(), rgb.g(), rgb.b(), a} {}
    constexpr vec(T r, const vec<T, 3>& gba): vec{r, gba.x(), gba.y(), gba.z()} {}
    explicit constexpr vec(const sf::Color& color): vec{color.r, color.g, color.b, color.a} {}
    constexpr vec& operator=(const sf::Color& color) {
        this->rgba(color.r, color.g, color.b, color.a);
        return *this;
    }

    void set(T r, T g, T b, T a) {
        this->rgba(r, g, b, a);
    }

    static vec from_str(const std::string& str_color) {
        static constexpr int hexmap[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0, 0, 10, 11, 12, 13, 14, 15};

        auto   p = str_color.data();
        vec color{255, 255, 255, 255};

        if (*p == '#') {
            ++p;

            for (int i = 0, v = toupper(*p); v && i < 8 && ((v >= '0' && v <= '9') || (v >= 'A' && v <= 'F')); ++i) {
                auto digit = uint8_t(hexmap[v - '0']);
                color.v[size_t(i) / 2] &= uint8_t(i % 2 ? (0xf0 | digit) : (0x0f | (digit << 4)));
                ++p;
                v = toupper(*p);
            }
        }

        return color;
    }

    std::string to_str() const {
        constexpr char hexmap[] = "0123456789abcdef";
        std::string    result   = "#";
        for (size_t i = 0; i < 3; ++i) {
            result.push_back(hexmap[v[i] >> 4]);
            result.push_back(hexmap[v[i] & 0xf]);
        }
        if (a() != 255) {
            result.push_back(hexmap[a() >> 4]);
            result.push_back(hexmap[a() & 0xf]);
        }
        return result;
    }
};

template <typename T, typename... Ts>
vec(T, Ts...) -> vec<T, sizeof...(Ts) + 1>;

template <typename T, size_t S>
vec(std::array<T, S>) -> vec<T, S>;

template <std::integral T1, std::integral T2, size_t S>
bool operator==(const vec<T1, S>& rhs, const vec<T2, S>& lhs) {
    return vec_integer_equal(rhs.v, lhs.v, std::make_index_sequence<S>());
}

template <std::integral T1, std::integral T2, size_t S>
bool operator!=(const vec<T1, S>& rhs, const vec<T2, S>& lhs) {
    return !(rhs == lhs);
}

/*
 * Aliases
 */

template <typename T>
using vec2 = vec<T, 2>;

template <typename T>
using vec3 = vec<T, 3>;

template <typename T>
using vec4 = vec<T, 4>;

using vec2i = vec2<int>;
using vec2u = vec2<unsigned>;
using vec2f = vec2<float>;
using vec2d = vec2<double>;

using vec3i = vec3<int>;
using vec3u = vec3<unsigned>;
using vec3f = vec3<float>;
using vec3d = vec3<double>;

using vec4i = vec4<int>;
using vec4u = vec4<unsigned>;
using vec4f = vec4<float>;
using vec4d = vec4<double>;

using rgba_t = vec4<uint8_t>;

template <size_t N, typename T, size_t S>
decltype(auto) get(const vec<T, S>& v) {
    return v.template get<N>();
}

template <size_t N, typename T, size_t S>
decltype(auto) get(vec<T, S>& v) {
    return v.template get<N>();
}

template <typename T, size_t S, typename F, size_t... Idxs>
constexpr inline auto vec_map(const vec<T, S>& n_dimm_vector, F callback, std::index_sequence<Idxs...>&&) {
    return vec{callback(std::get<Idxs>(n_dimm_vector.v))...};
}

template <typename T, size_t S, typename F>
constexpr inline auto vec_map(const vec<T, S>& n_dimm_vector, F callback) {
    return vec_map(n_dimm_vector, callback, std::make_index_sequence<S>());
}

template <std::floating_point T, size_t S>
constexpr inline auto round(const vec<T, S>& n_dimm_vector) {
    return vec_map(n_dimm_vector, [](auto v) { return std::round(v); });
}

template <typename T>
concept math_vector = requires { typename T::n_dimension_vector; };

template <math_vector T>
constexpr inline auto abs(const T& n_dim_vector) {
    using value_t = typename T::value_type;
    return vec_map(n_dim_vector, [](value_t v) { return static_cast<value_t>(std::abs(v)); });
}

template <math_vector T>
inline auto lerp(T v0, T v1, T t) {
    return v0 * (T::filled_with(1.f) - t) + v1 * t;
}
} // namespace core

namespace std
{
template <typename T, size_t S>
struct tuple_size<core::vec<T, S>> : std::integral_constant<size_t, S> {};

template <size_t N, typename T, size_t S>
struct tuple_element<N, core::vec<T, S>> : tuple_element<N, std::array<T, S>> {};
} // namespace std

#undef GEN_GET_2
#undef GEN_GET_3
#undef GEN_GET_2_FROM_VEC2
#undef GEN_GET_2_FROM_VEC3
#undef GEN_GET_2_FROM_VEC4
#undef GEN_GET_3_FROM_VEC3
#undef GEN_GET_3_FROM_VEC4
#undef GEN_GET_4_FROM_VEC4

#undef GEN_SET_2
#undef GEN_SET_3
#undef GEN_SET_2_FROM_VEC2
#undef GEN_SET_2_FROM_VEC3
#undef GEN_SET_2_FROM_VEC4
#undef GEN_SET_3_FROM_VEC3
#undef GEN_SET_3_FROM_VEC4
#undef GEN_SET_4_FROM_VEC4

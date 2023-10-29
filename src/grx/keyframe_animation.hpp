#pragma once

#include <vector>

#include "core/math.hpp"
#include "core/vec.hpp"

namespace grx
{
enum class interpolation_t { hold = 0, linear, bezier };
inline constexpr interpolation_t interpolation_not_set{-1};

template <typename T>
struct anim_key {
    T value;
    float time;
    interpolation_t interpolation;
    core::vec2f in;
    core::vec2f out;
};

template <typename T>
class anim_key_sequence {
public:
    void push(anim_key<T> key) {
        keys.push_back(std::move(key));
    }

    void push_hold(const T& value, float time) {
        push(anim_key<T>(value, time, interpolation_t::hold));
    }

    void push_linear(const T& value, float time) {
        push(anim_key<T>{value, time, interpolation_t::linear});
    }

    void push_linear_to_bezier(const T& value, float time, const core::vec2f& out) {
        push(anim_key<T>{value, time, interpolation_t::bezier, {0, 0}, out});
    }

    void push_bezier_to_linear(const T& value, float time, const core::vec2f& in) {
        push(anim_key<T>{value, time, interpolation_t::bezier, in, {1, 1}});
    }

    void push_bezier(const T& value, float time, const core::vec2f& in = {0, 0}, const core::vec2f& out = {1, 1}) {
        push(anim_key<T>{value, time, interpolation_t::bezier, in, out});
    }

    void normalize_time() {
        if (keys.empty())
            return;

        auto& max = keys.back().time;
        for (auto& key : keys)
            key.time = key.time / max;
    }

    T lookup(float time) const {
        if (keys.empty())
            return {};

        if (keys.size() == 1)
            return keys.back().value;

        auto i = keys.begin();
        while (i != keys.end() && i->time < time)
            ++i;

        auto k2 = i;
        if (k2 == keys.end())
            return keys.back().value;

        anim_key<T> k1;
        if (k2 == keys.begin())
            k1 = anim_key<T>{{}, 0, interpolation_t::linear};
        else
            k1 = *(k2 - 1);

        return interpolate(k1, *k2, time);
    }

private:
    static T interpolate(const anim_key<T>& k1, const anim_key<T>& k2, float time) {
        if (k2.interpolation == interpolation_t::hold)
            return k1.value;

        auto t = core::inverse_lerp(k1.time, k2.time, time);

        unsigned f = (unsigned(k1.interpolation == interpolation_t::bezier) << 1) |
                     unsigned(k2.interpolation == interpolation_t::bezier);

        switch (f) {
        /* k1 and k2 are linear */
        case 0: return core::lerp(k1.value, k2.value, t);
        /* k1 is linear, k2 is bezier */
        case 1: return core::cubic_bezier(k1.value, k2.value, 0.f, 0.f, k2.in.x(), k2.in.y(), t);
        /* k1 is bezier, k2 is linear */
        case 2: return core::cubic_bezier(k1.value, k2.value, k1.out.x(), k1.out.y(), 1.f, 1.f, t);
        /* k1 and k2 are bezier */
        case 3: return core::cubic_bezier(k1.value, k2.value, k1.out.x(), k1.out.y(), k2.in.x(), k2.in.y(), t);
        }

        return {};
    }

private:
    std::vector<anim_key<T>> keys;
};
}; // namespace grx

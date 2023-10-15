#pragma once

#include <vector>

#include "core/math.hpp"


namespace grx
{
enum class Interpolation { hold = 0, linear, bezier };

struct EaseVec2 {
    float x, y;
};

template <typename T>
struct AnimKey {
    T value;
    float time;
    Interpolation interpolation;
    EaseVec2 in;
    EaseVec2 out;
};

template <typename T>
class AnimKeySequence {
public:
    void push(const AnimKey<T>& key) {
        keys.push_back(key);
    }

    void push_hold(const T& value, float time) {
        push(AnimKey<T>(value, time, Interpolation::hold));
    }

    void push_linear(const T& value, float time) {
        push(AnimKey<T>{value, time, Interpolation::linear});
    }

    void push_linear_to_bezier(const T& value, float time, const EaseVec2& out) {
        push(AnimKey<T>{value, time, Interpolation::bezier, {0, 0}, out});
    }

    void push_bezier_to_linear(const T& value, float time, const EaseVec2& in) {
        push(AnimKey<T>{value, time, Interpolation::bezier, in, {1, 1}});
    }

    void push_bezier(const T& value, float time, const EaseVec2& in = {0, 0}, const EaseVec2& out = {1, 1}) {
        push(AnimKey<T>{value, time, Interpolation::bezier, in, out});
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

        AnimKey<T> k1;
        if (k2 == keys.begin())
            k1 = AnimKey<T>{{}, 0, Interpolation::linear};
        else
            k1 = *(k2 - 1);

        return interpolate(k1, *k2, time);
    }

private:
    static T interpolate(const AnimKey<T>& k1, const AnimKey<T>& k2, float time) {
        if (k2.interpolation == Interpolation::hold)
            return k1.value;

        auto t      = core::inverse_lerp(k1.time, k2.time, time);
        auto inv_t  = 1.f - t;
        auto t2     = t * t;
        auto t3     = t2 * t;
        auto inv_t2 = inv_t * inv_t;
        auto inv_t3 = inv_t2 * inv_t;

        unsigned f = (unsigned(k1.interpolation == Interpolation::bezier) << 1) |
                     unsigned(k2.interpolation == Interpolation::bezier);

        switch (f) {
        /* k1 and k2 are linear */
        case 0: return core::lerp(k1.value, k2.value, t);
        /* k1 is linear, k2 is bezier */
        case 1: return core::cubic_bezier(k1.value, k2.value, 0.f, 0.f, k2.in.x, k2.in.y, t);
        /* k1 is bezier, k2 is linear */
        case 2: return core::cubic_bezier(k1.value, k2.value, k1.out.x, k1.out.y, 1.f, 1.f, t);
        /* k1 and k2 are bezier */
        case 3: return core::cubic_bezier(k1.value, k2.value, k1.out.x, k1.out.y, k2.in.x, k2.in.y, t);
        }

        return {};
    }

private:
    std::vector<AnimKey<T>> keys;
};
}; // namespace grx

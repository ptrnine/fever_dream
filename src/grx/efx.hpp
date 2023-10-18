#pragma once

#include <functional>
#include <list>

#include "core/math.hpp"
#include "core/vec.hpp"
#include "keyframe_animation.hpp"
#include "scene.hpp"

namespace grx
{
struct efx_state {
    const scene::batch* batch;
    float               timestep;
    float               timestep_coef;
    float               time_elapsed;
    float               time_elapsed_coef;
    uint32_t            idx;
};

class efx {
public:
    friend class efx_instance;

    class handler_t {
    public:
        using index_t = uint32_t;

        template <typename F>
        handler_t(F&& function): handler(std::forward<F>(function)) {}

        void set_affected_indices(std::initializer_list<index_t> indices) {
            affects_all = false;
            affected_indices.assign(indices);
        }

        void set_affected_indices(std::vector<index_t> indices) {
            affects_all = false;
            affected_indices = std::move(indices);
        }

        void set_affects_all(bool value = true) {
            affects_all = value;
        }

        const auto& get_affected_indices() const {
            return affected_indices;
        }

        void operator()(auto& drawables, efx_state state) {
            if (affects_all) {
                for (size_t idx = 0; idx < drawables.size(); ++idx) {
                    state.idx = idx;
                    handler(drawables[idx], state);
                }
            }
            else {
                for (auto idx : affected_indices) {
                    state.idx = idx;
                    handler(drawables[idx], state);
                }
            }
        }

    private:
        std::vector<index_t>                               affected_indices;
        std::function<void(drawable_t&, const efx_state&)> handler;
        bool                                               affects_all = true;
    };

    template <typename F>
    handler_t& add_handler(const std::string& name, F&& function) {
        return handlers
            .insert_or_assign(name,
                              [f = std::forward<F>(function)](drawable_t& drawable, const efx_state& state) mutable {
                                  downcast(drawable, std::forward<F>(f), state);
                              })
            .first->second;
    }

    handler_t* get_handler(const std::string& name) {
        auto bucket = handlers.find(name);
        if (bucket == handlers.end())
            return nullptr;
        return &bucket->second;
    }

    const handler_t* get_handler(const std::string& name) const {
        auto bucket = handlers.find(name);
        if (bucket == handlers.end())
            return nullptr;
        return &bucket->second;
    }

    auto& get_elements() {
        return elements;
    }

    const auto& get_elements() const {
        return elements;
    }

    void set_duration(float value) {
        duration = value;
    }

    float get_duration() const {
        return duration;
    }

    template <typename T>
    T& create_element(T&& element) {
        elements.emplace_back(std::forward<T>(element));
        return std::get<std::decay_t<T>>(elements.back());
    }

    drawable_t& push_element(drawable_t element) {
        elements.emplace_back(std::move(element));
        return elements.back();
    }

private:
    std::vector<drawable_t>          elements;
    std::map<std::string, handler_t> handlers;
    float                            duration;
};

static inline constexpr float duration_endless = std::numeric_limits<float>::infinity();

class efx_instance {
public:
    efx_instance(scene& scene, scene::layer_t layer, efx& iefx):
        e(&iefx), batch(scene.create_batch(layer, e->get_elements())), duration(e->get_duration()) {
        batch.delete_later();
        for (auto&& [_, handler] : e->handlers) handlers.push_back(handler);
    }

    void set_batch(const scene::batch_ref& ibatch) {
        batch = ibatch;
    }

    void set_duration(float value) {
        duration = value;
    }

    void update(float timestep) {
        for (auto&& handler : handlers)
            handler(batch->get_elements(),
                    {
                        .batch             = batch.get_pointer(),
                        .timestep          = timestep,
                        .timestep_coef     = timestep / duration,
                        .time_elapsed      = time_elapsed,
                        .time_elapsed_coef = time_elapsed / duration,
                    });
        time_elapsed += timestep;
    }

    bool timeout() const {
        return time_elapsed >= duration;
    }

    void move(const core::vec2f& movement) {
        batch->move(movement);
    }

    void scale(const core::vec2f& scale) {
        batch->scale(scale);
    }

    auto& get_elements() {
        return batch->get_elements();
    }

    const auto& get_elements() const {
        return batch->get_elements();
    }

private:
    std::vector<efx::handler_t> handlers;
    efx*                        e;
    scene::batch_ref            batch;
    float                       duration;
    float                       time_elapsed = 0.f;
};

class efx_mgr {
public:
    efx_mgr(scene& iscene): s(&iscene) {}

    void add_effect(const std::string& name, efx effect) {
        effects.insert_or_assign(name, std::move(effect));
    }

    bool play(const std::string& name,
              scene::layer_t     layer,
              const core::vec2f& position = {0, 0},
              const core::vec2f& scale    = {1.f, 1.f}) {
        auto found = effects.find(name);
        if (found == effects.end())
            return false;

        running_effects.push_back(efx_instance(*s, layer, found->second));
        auto& instance = running_effects.back();
        instance.move(position);
        instance.scale(scale);

        return true;
    }

    void update(float timestep) {
        for (auto i = running_effects.begin(); i != running_effects.end();) {
            if (i->timeout())
                running_effects.erase(i++);
            else
                i++->update(timestep);
        }
    }

private:
    scene*                     s;
    std::map<std::string, efx> effects;
    std::list<efx_instance>    running_effects;
};

namespace efx_handlers
{
    inline auto position(const anim_key_sequence<core::vec2f>& keys) {
        return [keys = keys](sf::Transformable& obj, const efx_state& state) {
            obj.setPosition(keys.lookup(state.time_elapsed_coef));
        };
    }

    inline auto scale(const anim_key_sequence<core::vec2f>& keys) {
        return [keys = keys](sf::Transformable& obj, const efx_state& state) {
            obj.setScale(keys.lookup(state.time_elapsed_coef));
        };
    }

    inline auto rotation(const anim_key_sequence<float>& keys) {
        return [keys = keys](sf::Transformable& obj, const efx_state& state) {
            obj.setRotation(keys.lookup(state.time_elapsed_coef));
        };
    }

    inline auto gravity(const std::vector<float>& masses = {}, const std::vector<core::vec2f>& velocities = {}) {
        return [masses = masses, velocities = velocities](sf::Transformable& obj, const efx_state& state) mutable {
            auto  idx    = state.idx;
            auto& bodies = state.batch->get_elements();

            if (bodies.size() > masses.size())
                masses.resize(bodies.size(), 1.f);
            if (bodies.size() > velocities.size())
                velocities.resize(bodies.size(), {0, 0});

            core::vec2f accel{0, 0};

            for (size_t i = 0; i < bodies.size(); ++i) {
                if (i == idx)
                    continue;
                auto mass = masses[i];
                auto pos  = std::visit([](auto&& obj) { return obj.getPosition(); }, bodies[i]);
                auto dir  = core::vec2f(pos - obj.getPosition()).normalize();
                accel += dir * mass;
            }

            auto& velocity = velocities[idx];
            velocity += accel * state.timestep;
            obj.move(velocity * state.timestep);
        };
    }
}; // namespace efx_handlers
} // namespace grx

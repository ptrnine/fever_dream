#pragma once

#include <functional>
#include <list>

#include "core/math.hpp"
#include "core/vec.hpp"
#include "keyframe_animation.hpp"
#include "scene.hpp"

namespace grx
{
struct ParticleState {
    const Scene::Batch* batch;
    float timestep;
    float timestep_coef;
    float time_elapsed;
    float time_elapsed_coef;
    uint32_t idx;
};


class Particles {
public:
    friend class ParticlesInstance;

    class Handler {
    public:
        using index_t = uint32_t;

        template <typename F>
        Handler(F&& function): handler(std::forward<F>(function)) {}

        void set_affected_indices(std::initializer_list<index_t> indices) {
            affects_all = false;
            affected_indices.assign(indices);
        }

        void set_affects_all(bool value = true) {
            affects_all = value;
        }

        const auto& get_affected_indices() const {
            return affected_indices;
        }

        void operator()(auto& drawables, ParticleState state) {
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
        std::vector<index_t> affected_indices;
        std::function<void(Drawable&, const ParticleState&)> handler;
        bool affects_all = true;
    };

    template <typename F>
    Handler& add_handler(const std::string& name, F&& function) {
        return handlers
            .insert_or_assign(name,
                              [f = std::forward<F>(function)](Drawable& drawable, const ParticleState& state) mutable {
                                  downcast(drawable, std::forward<F>(f), state);
                              })
            .first->second;
    }

    Handler* get_handler(const std::string& name) {
        auto bucket = handlers.find(name);
        if (bucket == handlers.end())
            return nullptr;
        return &bucket->second;
    }

    const Handler* get_handler(const std::string& name) const {
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

private:
    std::vector<Drawable> elements;
    std::map<std::string, Handler> handlers;
    float duration;
};


static inline constexpr float duration_endless = std::numeric_limits<float>::infinity();

class ParticlesInstance {
public:
    ParticlesInstance(Scene& scene, Scene::layer_t layer, Particles& iparticles):
        particles(&iparticles),
        batch(scene.create_batch(layer, particles->get_elements())),
        duration(particles->get_duration()) {
        batch.delete_later();
        for (auto&& [_, handler] : particles->handlers)
            handlers.push_back(handler);
    }

    void set_batch(const Scene::BatchRef& ibatch) {
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
    std::vector<Particles::Handler> handlers;
    Particles* particles;
    Scene::BatchRef batch;
    float duration;
    float time_elapsed = 0.f;
};


class ParticlesMgr {
public:
    ParticlesMgr(Scene& iscene): scene(&iscene) {}

    void add_effect(const std::string& name, Particles effect) {
        effects.insert_or_assign(name, std::move(effect));
    }

    bool play(const std::string& name,
              Scene::layer_t     layer,
              const core::vec2f& position = {0, 0},
              const core::vec2f& scale    = {1.f, 1.f}) {
        auto found = effects.find(name);
        if (found == effects.end())
            return false;

        running_effects.push_back(ParticlesInstance(*scene, layer, found->second));
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
    Scene* scene;
    std::map<std::string, Particles> effects;
    std::list<ParticlesInstance> running_effects;
};

namespace particle {
    inline auto position(const AnimKeySequence<core::vec2f>& keys) {
        return [keys = keys](sf::Transformable& obj, const ParticleState& state) {
            obj.setPosition(keys.lookup(state.time_elapsed_coef));
        };
    }

    inline auto scale(const AnimKeySequence<core::vec2f>& keys) {
        return [keys = keys](sf::Transformable& obj, const ParticleState& state) {
            obj.setScale(keys.lookup(state.time_elapsed_coef));
        };
    }

    inline auto rotation(const AnimKeySequence<float>& keys) {
        return [keys = keys](sf::Transformable& obj, const ParticleState& state) {
            obj.setRotation(keys.lookup(state.time_elapsed_coef));
        };
    }

    inline auto gravity(const std::vector<float>& masses = {}, const std::vector<core::vec2f>& velocities = {}) {
        return [masses = masses, velocities = velocities](sf::Transformable& obj, const ParticleState& state) mutable {
            auto idx = state.idx;
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
                auto pos = std::visit([](auto&& obj) { return obj.getPosition(); }, bodies[i]);
                auto dir = core::vec2f(pos - obj.getPosition()).normalize();
                accel += dir * mass;
            }

            auto& velocity = velocities[idx];
            velocity += accel * state.timestep;
            obj.move(velocity * state.timestep);
        };
    }
};    // namespace particle
} // namespace grx

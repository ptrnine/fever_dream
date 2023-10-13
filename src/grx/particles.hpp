#pragma once

#include <functional>
#include <list>

#include "scene.hpp"
#include "../core/math.hpp"

namespace grx {
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

    void update(Scene::BatchRef& batch, const ParticleState& state) {
        for (auto&& [_, handler] : handlers)
            handler(batch->get_elements(), state);
    }

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

private:
    std::vector<Drawable> elements;
    std::map<std::string, Handler> handlers;
    float duration;
};


static inline constexpr float duration_endless = std::numeric_limits<float>::infinity();

class ParticlesInstance {
public:
    ParticlesInstance(Scene& scene, Scene::layer_t layer, Particles& iparticles):
        particles(&iparticles), batch(scene.create_batch(layer, particles->get_elements())),
        duration(particles->get_duration()) {
        batch.delete_later();
    }

    void set_batch(const Scene::BatchRef& ibatch) {
        batch = ibatch;
    }

    void set_duration(float value) {
        duration = value;
    }

    void update(float timestep) {
        particles->update(batch,
                          {
                              .batch = batch.get_pointer(),
                              .timestep = timestep,
                              .timestep_coef = timestep / duration,
                              .time_elapsed = time_elapsed,
                              .time_elapsed_coef = time_elapsed / duration,
                          });
        time_elapsed += timestep;
    }

    bool timeout() const {
        return time_elapsed >= duration;
    }

    auto& get_elements() {
        return batch->get_elements();
    }

    const auto& get_elements() const {
        return batch->get_elements();
    }

private:
    Particles* particles;
    Scene::BatchRef batch;
    float time_elapsed;
    float duration;
};


class ParticlesMgr {
public:
    ParticlesMgr(Scene& iscene): scene(&iscene) {}

    void add_effect(const std::string& name, Particles effect) {
        effects.insert_or_assign(name, std::move(effect));
    }

    bool play(const std::string& name, Scene::layer_t layer, const sf::Vector2f& position = {0, 0}) {
        auto found = effects.find(name);
        if (found == effects.end())
            return false;

        running_effects.push_back(ParticlesInstance(*scene, layer, found->second));
        auto& instance = running_effects.back();
        for (auto&& element : instance.get_elements())
            std::visit([=](sf::Transformable& obj) { obj.move(position); }, element);

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
    namespace details {
        inline auto value_change(float max_movement, auto dependence) {
            return [=, prev_t = 0.f](const ParticleState& state) mutable {
                auto t = dependence(state.time_elapsed_coef);
                auto step = t - prev_t;
                prev_t = t;
                return max_movement * step;
            };
        }
    } // namespace details

    template <typename F = decltype(core::dependence::linear<float>)>
    inline auto x_movement(float max_movement, F dependence = core::dependence::linear<float>) {
        return [=, f = details::value_change(max_movement, dependence)](sf::Transformable& obj,
                                                                        const ParticleState& state) mutable {
            obj.move(f(state), 0);
        };
    }

    template <typename F = decltype(core::dependence::linear<float>)>
    inline auto y_movement(float max_movement, F dependence = core::dependence::linear<float>) {
        return [=, f = details::value_change(max_movement, dependence)](sf::Transformable& obj,
                                                                        const ParticleState& state) mutable {
            obj.move(0, f(state));
        };
    }

    template <typename F = decltype(core::dependence::linear<float>)>
    inline auto xy_movement(sf::Vector2f max_movement, F dependence = core::dependence::linear<float>) {
        return [=,
                fx = details::value_change(max_movement.x, dependence),
                fy = details::value_change(max_movement.y, dependence)](sf::Transformable& obj,
                                                                        const ParticleState& state) mutable {
            obj.move(fx(state), fy(state));
        };
    }

    inline auto gravity(const std::vector<float>& masses = {}, const std::vector<sf::Vector2f>& velocities = {}) {
        return [masses = masses, velocities = velocities](sf::Transformable& obj, const ParticleState& state) mutable {
            auto idx = state.idx;
            auto& bodies = state.batch->get_elements();

            if (bodies.size() > masses.size())
                masses.resize(bodies.size(), 1.f);
            if (bodies.size() > velocities.size())
                velocities.resize(bodies.size(), {0, 0});

            sf::Vector2f accel{0, 0};

            for (size_t i = 0; i < bodies.size(); ++i) {
                if (i == idx)
                    continue;
                auto mass = masses[i];
                auto pos = std::visit([](auto&& obj) { return obj.getPosition(); }, bodies[i]);
                auto dir = core::normalize(pos - obj.getPosition());
                accel += dir * mass;
            }

            auto& velocity = velocities[idx];
            velocity += accel * state.timestep;
            obj.move(velocity * state.timestep);
        };
    }
};    // namespace particle
} // namespace grx

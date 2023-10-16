#pragma once

#include <iostream>

#include <cstdint>
#include <map>
#include <vector>

#include <SFML/Graphics/RenderTarget.hpp>

#include "core/vec.hpp"
#include "sfml_types.hpp"

namespace grx
{

class scene {
public:
    using layer_t = uint64_t;
    using id_t    = uint64_t;

    static inline constexpr auto empty_id = std::numeric_limits<id_t>::max();

    class batch {
    public:
        friend scene;

        batch(layer_t ilayer = 0): layer(ilayer) {}

        batch(batch&&)            = default;
        batch& operator=(batch&&) = default;

        layer_t get_layer() const {
            return layer;
        }

        void set_elements(auto&& input_elements) {
            elements.assign(input_elements.begin(), input_elements.end());
        }

        auto& get_elements() {
            return elements;
        }

        const auto& get_elements() const {
            return elements;
        }

        template <typename T, typename... Args>
        decltype(auto) create_element(Args&&... args) {
            elements.push_back({});
            return elements.back().emplace<T>(std::forward<Args>(args)...);
        }

        void draw(const scene*      scene,
                  sf::RenderTarget& target,
                  sf::RenderStates  render_states = sf::RenderStates::Default) const {
            if (elements.empty())
                return;

            auto final_transform = calc_final_transform(scene);
            render_states.transform.combine(final_transform);
            for (auto element : elements)
                downcast(element, [&](const sf::Drawable& drawable) { target.draw(drawable, render_states); });
        }

        auto get_users() const {
            return users;
        }

        void move(const core::vec2f& movement) {
            transform.translate(movement);
        }

        void scale(const core::vec2f& scale) {
            /* TODO: optimize for 0 0 scale */
            transform.scale(scale, calc_center());
        }

        core::vec2f calc_center() {
            core::vec2f center{0, 0};
            for (auto&& element : elements)
                center +=
                    std::visit([](auto&& obj) { return obj.getPosition(); }, element) * (1.f / float(elements.size()));
            return center;
        }

    private:
        void increment_users() {
            ++users;
        }

        void decrement_users() {
            --users;
        }

        sf::Transform calc_final_transform(const scene* scene) const {
            if (parent_id != empty_id) {
                auto parent = scene->get_batch_pointer(parent_id);
                if (parent)
                    return parent->calc_final_transform(scene) * transform;
            }
            return transform;
        }

    private:
        layer_t                 layer;
        std::vector<drawable_t> elements;
        sf::Transform           transform    = sf::Transform::Identity;
        id_t                    parent_id    = empty_id;
        uint32_t                users        = 0;
        bool                    delete_later = false;
    };

    using batch_storage_t = std::map<id_t, batch>;

    class batch_ref;

    class item_ref {
    public:
        friend scene;

        item_ref() = default;

        item_ref(const item_ref& item): s(item.s), cached(item.cached), id(item.id) {
            increment_users();
        }

        item_ref& operator=(const item_ref& item) {
            if (&item == this)
                return *this;

            destroy();

            s      = item.s;
            cached = item.cached;
            id     = item.id;

            increment_users();

            return *this;
        }

        item_ref(item_ref&& item) noexcept: s(item.s), cached(item.cached), id(item.id) {
            item.cached = nullptr;
            item.s      = nullptr;
        }

        item_ref& operator=(item_ref&& item) noexcept {
            if (&item == this)
                return *this;

            destroy();

            s      = item.s;
            cached = item.cached;
            id     = item.id;

            item.cached = nullptr;
            item.s      = nullptr;

            return *this;
        }

        ~item_ref() {
            destroy();
        }

        void delete_later(bool value = true) {
            get_pointer()->delete_later = value;
        }

        id_t get_id() const {
            return id;
        }

        scene* get_scene() const {
            return s;
        }

        layer_t get_layer() const {
            return get_pointer()->get_layer();
        }

        batch* get_pointer() {
            if (!cached)
                return cached = s->get_batch_pointer(id);
            return cached;
        }

        const batch* get_pointer() const {
            if (!cached)
                return cached = s->get_batch_pointer(id);
            return cached;
        }

        void set_parent(const item_ref& item) {
            get_pointer()->parent_id = item.id;
        }

        batch_ref get_parent();

        explicit operator bool() const {
            return s;
        }

    protected:
        void destroy() {
            // std::cout << "destroy [batch:" << id << "]" << std::endl;
            if (!s)
                return;

            if (auto p = get_pointer()) {
                p->decrement_users();
                if (p->delete_later && p->users == 0)
                    s->delete_item(id);
            }
        }

        void increment_users() {
            if (s)
                if (auto p = get_pointer())
                    p->increment_users();
        }

        item_ref(scene* iscene, batch* batch, id_t iid): s(iscene), cached(batch), id(iid) {
            get_pointer()->increment_users();
        }

    private:
        scene*         s      = nullptr;
        mutable batch* cached = nullptr;
        id_t           id;
    };

    class batch_ref : public item_ref {
    public:
        friend class scene;
        friend item_ref;

        batch_ref(): item_ref() {}

        batch* operator->() {
            return get_pointer();
        }

        const batch* operator->() const {
            return get_pointer();
        }

        batch& operator*() {
            return *get_pointer();
        }

        const batch& operator*() const {
            return *get_pointer();
        }

    private:
        batch_ref(class scene* scene, batch* batch, id_t id): item_ref(scene, batch, id) {}
    };

    template <typename T>
    class element_ref : public item_ref {
    public:
        friend class scene;

        element_ref(): item_ref() {}

        T* operator->() {
            return &operator*();
        }

        const T* operator->() const {
            return &operator*();
        }

        T& operator*() {
            return std::get<T>(get_pointer()->get_elements().front());
        }

        const T& operator*() const {
            return std::get<T>(get_pointer()->get_elements().front());
        }

    private:
        element_ref(class scene* scene, batch* batch, id_t id): item_ref(scene, batch, id) {}
    };

    void draw(sf::RenderTarget& target, const sf::RenderStates& render_states = sf::RenderStates::Default) const {
        for (auto [layer, _] : layers_usage)
            for (auto&& [_, batch] : batches)
                if (layer == batch.layer)
                    batch.draw(this, target, render_states);
    }

    template <typename T, typename... Args>
    element_ref<T> create_element(layer_t layer = 0, Args&&... args) {
        auto id = next_id();

        batch batch{layer};
        batch.create_element<T>(std::forward<Args>(args)...);

        auto [bucket, _] = batches.emplace(id, std::move(batch));
        ++layers_usage[layer];

        return {this, bucket, id};
    }

    batch_ref create_batch(layer_t layer = 0) {
        auto id          = next_id();
        auto [bucket, _] = batches.emplace(id, layer);
        ++layers_usage[layer];
        return {this, &bucket->second, id};
    }

    batch_ref create_batch(layer_t layer, auto&& drawables) {
        auto batch = create_batch(layer);
        batch->set_elements(drawables);
        return batch;
    }

    bool delete_item(id_t id) {
        auto bucket = batches.find(id);
        if (bucket == batches.end())
            return false;

        auto layer = bucket->second.layer;
        batches.erase(bucket);
        --layers_usage[layer];
        return true;
    }

    bool delete_item(const item_ref& item) {
        return delete_item(item.get_id());
    }

    size_t get_batches_count() const {
        return batches.size();
    }

    size_t get_elements_count() const {
        size_t result = 0;
        for (auto&& batch : batches) result += batch.second.elements.size();
        return result;
    }

    batch_ref get_batch(id_t id) {
        if (auto p = get_batch_pointer(id); p)
            return {this, p, id};
        return {};
    }

private:
    id_t next_id() {
        return id_counter++;
    }

    batch* get_batch_pointer(id_t id) {
        auto bucket = batches.find(id);
        if (bucket != batches.end())
            return &bucket->second;
        return nullptr;
    }

    const batch* get_batch_pointer(id_t id) const {
        auto bucket = batches.find(id);
        if (bucket != batches.end())
            return &bucket->second;
        return nullptr;
    }

private:
    std::map<id_t, batch>       batches;
    std::map<layer_t, uint64_t> layers_usage;
    id_t                        id_counter = 0;
};

inline scene::batch_ref scene::item_ref::get_parent() {
    return s->get_batch(get_pointer()->parent_id);
}
} // namespace grx

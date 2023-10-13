#pragma once

#include <iostream>

#include <cstdint>
#include <map>
#include <vector>

#include <SFML/Graphics/RenderTarget.hpp>

#include "sfml_types.hpp"

namespace grx {

class Scene {
public:
    using layer_t = uint64_t;
    using id_t = uint64_t;

    friend class ItemRef;
    friend class BatchRef;
    friend class ElementRef;


    class Batch {
    public:
        friend Scene;

        Batch(layer_t ilayer = 0): layer(ilayer) {}

        Batch(Batch&&) = default;
        Batch& operator=(Batch&&) = default;

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

    private:
        layer_t layer;
        std::vector<Drawable> elements;
        uint32_t users = 0;
        bool delete_later = false;
    };


    using batch_storage_t = std::map<id_t, Batch>;


    class ItemRef {
    public:
        friend Scene;

        ItemRef() = default;

        ItemRef(const ItemRef& item): scene(item.scene), cached(item.cached), id(item.id) {
            if (auto p = get_pointer())
                ++p->users;
        }

        ItemRef& operator=(const ItemRef& item) {
            if (&item == this)
                return *this;

            if (auto p = get_pointer()) {
                --p->users;
                if (p->delete_later && p->users == 1)
                    scene->delete_item(id);
            }

            scene = item.scene;
            cached = item.cached;
            id = item.id;

            if (auto p = get_pointer())
                ++p->users;

            return *this;
        }

        ItemRef(ItemRef&& item) noexcept: scene(item.scene), cached(item.cached), id(item.id) {
            item.cached = nullptr;
        }

        ItemRef& operator=(ItemRef&& item) noexcept {
            if (&item == this)
                return *this;

            scene = item.scene;
            cached = item.cached;
            id = item.id;

            item.cached = nullptr;

            return *this;
        }

        ~ItemRef() {
            if (auto p = get_pointer()) {
                --p->users;
                if (p->delete_later && p->users == 1)
                    scene->delete_item(id);
            }
        }

        void delete_later(bool value = true) {
            get_pointer()->delete_later = value;
        }

        id_t get_id() const {
            return id;
        }

        Scene* get_scene() const {
            return scene;
        }

        layer_t get_layer() const {
            return get_pointer()->get_layer();
        }

        Batch* get_pointer() {
            if (!cached)
                return cached = scene->get_batch_pointer(id);
            return cached;
        }

        const Batch* get_pointer() const {
            if (!cached)
                return cached = scene->get_batch_pointer(id);
            return cached;
        }

    protected:
        ItemRef(Scene* iscene, Batch* batch, id_t iid): scene(iscene), cached(batch), id(iid) {}

    private:
        Scene* scene = nullptr;
        mutable Batch* cached = nullptr;
        id_t id;
    };


    class BatchRef : public ItemRef {
    public:
        friend Scene;

        BatchRef(): ItemRef() {}

        Batch* operator->() {
            return get_pointer();
        }

        const Batch* operator->() const {
            return get_pointer();
        }

        Batch& operator*() {
            return *get_pointer();
        }

        const Batch& operator*() const {
            return *get_pointer();
        }

    private:
        BatchRef(Scene* scene, Batch* batch, id_t id): ItemRef(scene, batch, id) {}
    };


    template <typename T>
    class ElementRef : public ItemRef {
    public:
        friend Scene;

        ElementRef(): ItemRef() {}

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
        ElementRef(Scene* scene, Batch* batch, id_t id): ItemRef(scene, batch, id) {}
    };


    void draw(sf::RenderTarget& target, const sf::RenderStates& render_states = sf::RenderStates::Default) const {
        for (auto [layer, _] : layers_usage)
            for (auto&& [_, batch] : batches)
                if (layer == batch.layer)
                    for (auto element : batch.elements)
                        downcast(element, [&](const sf::Drawable& drawable) { target.draw(drawable, render_states); });
    }

    template <typename T, typename... Args>
    ElementRef<T> create_element(layer_t layer = 0, Args&&... args) {
        auto id = next_id();

        Batch batch{layer};
        batch.create_element<T>(std::forward<Args>(args)...);

        auto [bucket, _] = batches.emplace(id, std::move(batch));
        ++layers_usage[layer];

        return {this, bucket, id};
    }

    BatchRef create_batch(layer_t layer = 0) {
        auto id = next_id();
        auto [bucket, _] = batches.emplace(id, layer);
        ++layers_usage[layer];
        return {this, &bucket->second, id};
    }

    BatchRef create_batch(layer_t layer, auto&& drawables) {
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

    bool delete_item(const ItemRef& item) {
        return delete_item(item.get_id());
    }

    size_t get_batches_count() const {
        return batches.size();
    }

    size_t get_elements_count() const {
        size_t result = 0;
        for (auto&& batch : batches)
            result += batch.second.elements.size();
        return result;
    }

private:
    id_t next_id() {
        return id_counter++;
    }

    Batch* get_batch_pointer(id_t id) {
        auto bucket = batches.find(id);
        if (bucket != batches.end())
            return &bucket->second;
        return nullptr;
    }

private:
    std::map<id_t, Batch> batches;
    std::map<layer_t, uint64_t> layers_usage;
    id_t id_counter = 0;
};
} // namespace grx

#pragma once

#include <iostream>

#include <cstdint>
#include <map>
#include <vector>

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

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

        ~Batch() {
            for (auto element : elements)
                delete element;
        }

        layer_t get_layer() const {
            return layer;
        }

        auto& get_elements() {
            return elements;
        }

        template <typename T, typename... Args>
        T* create_element(Args&&... args) {
            auto object = new T(std::forward<Args>(args)...);
            elements.push_back(object);
            return object;
        }

    private:
        layer_t layer;
        std::vector<sf::Drawable*> elements;
    };


    using batch_storage_t = std::map<id_t, Batch>;
    using batch_bucket_t = batch_storage_t::iterator;


    class ItemRef {
    public:
        friend Scene;

        ~ItemRef() {
            if (to_delete)
                scene->delete_item(id);
        }

        void delete_later(bool value = true) {
            to_delete = value;
        }

        id_t get_id() const {
            return id;
        }

        Scene* get_scene() const {
            return scene;
        }

        layer_t get_layer() const {
            return access_pointer()->get_layer();
        }

    protected:
        ItemRef(Scene* iscene, batch_bucket_t ibucket, id_t iid): scene(iscene), cached_bucket(ibucket), id(iid) {}

        Batch* access_pointer() const {
            if (scene->is_empty_bucket(cached_bucket)) {
                cached_bucket = scene->access_bucket(id);
                if (scene->is_empty_bucket(cached_bucket))
                    return nullptr;
            }
            return &cached_bucket->second;
        }

    private:
        Scene* scene;
        mutable batch_bucket_t cached_bucket;
        id_t id;
        bool to_delete = false;
    };


    class BatchRef : public ItemRef {
    public:
        friend Scene;

        Batch* operator->() {
            return access_pointer();
        }

        Batch& operator*() {
            return *access_pointer();
        }

    private:
        BatchRef(Scene* scene, batch_bucket_t bucket, id_t id): ItemRef(scene, bucket, id) {}
    };


    template <typename T>
    class ElementRef : public ItemRef {
    public:
        friend Scene;

        T* operator->() {
            return static_cast<T*>(access_pointer()->get_elements().front());
        }

        Batch& operator*() {
            return *operator->();
        }

    private:
        ElementRef(Scene* scene, batch_bucket_t bucket, id_t id): ItemRef(scene, bucket, id) {}
    };


    void draw(sf::RenderTarget& target, const sf::RenderStates& render_states = sf::RenderStates::Default) const {
        for (auto [layer, _] : layers_usage)
            for (auto&& [_, batch] : batches)
                if (layer == batch.layer)
                    for (auto element : batch.elements)
                        target.draw(*element, render_states);
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
        return {this, bucket, id};
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

private:
    id_t next_id() {
        return id_counter++;
    }

    batch_bucket_t access_bucket(id_t id) {
        return batches.find(id);
    }

    bool is_empty_bucket(const batch_bucket_t& bucket) const {
        return bucket == batches.end();
    }

private:
    std::map<id_t, Batch> batches;
    std::map<layer_t, uint64_t> layers_usage;
    id_t id_counter = 0;
};
} // namespace grx

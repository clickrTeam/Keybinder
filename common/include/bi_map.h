#pragma once

#include <QHash>
#include <stdexcept>

template <typename Key, typename Value> class BiMap {
  public:
    BiMap() = default;
    BiMap(std::initializer_list<std::pair<Key, Value>> list) {
        for (const auto &pair : list) {
            insert(pair.first, pair.second);
        }
    }

    void insert(const Key &key, const Value &value) {
        forward_map.insert(key, value);
        backward_map.insert(value, key);
    }

    Value find_forward(const Key &key) const {
        auto it = forward_map.find(key);
        if (it != forward_map.end()) {
            return *it;
        }
        throw std::out_of_range("Key not found in forward map.");
    }

    bool contains_forward(const Key &key) const {
        return forward_map.contains(key);
    }

    Key find_backward(const Value &value) const {
        auto it = backward_map.find(value);
        if (it != backward_map.end()) {
            return *it;
        }
        throw std::out_of_range("Value not found in backward map.");
    }
    bool contains_backwards(const Key &key) const {
        return backward_map.contains(key);
    }

  private:
    QHash<Key, Value> forward_map;
    QHash<Value, Key> backward_map;
};

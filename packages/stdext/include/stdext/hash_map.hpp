#pragma once

#include "crypto.hpp"
#include <initializer_list>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace stdext {
    template<typename Key>
    struct hash_map_default_hasher {
        auto operator()(const Key& key) const {
            if constexpr (std::is_same_v<Key, std::string>) {
                return crypto::hash::fnv(key.data(), static_cast<int>(key.size()));
            }
            else if constexpr (std::is_pointer_v<Key>) {
                return reinterpret_cast<std::uintptr_t>(key);
            }
            else {
                return key;
            }
        }
    };

    template<typename Key, typename Value, typename Hasher = hash_map_default_hasher<Key>>
    class hash_map {
    private:
        struct Slot {
            Key key;
            Value value;
            bool occupied = false;
            bool deleted = false;

            Slot() : key(), value(), occupied(false), deleted(false) {}
        };

        template<bool IsConst>
        class basic_iterator {
        public:
            using SlotPtr = std::conditional_t<IsConst, const std::vector<Slot>*, std::vector<Slot>*>;
            using ValueRef = std::conditional_t<IsConst, const Value&, Value&>;

            struct Entry {
                const Key& first;
                ValueRef second;
            };

            struct ArrowProxy {
                Entry entry;

                Entry* operator->() {
                    return &entry;
                }
            };

            basic_iterator(SlotPtr slots, size_t index) : m_slots(slots), m_index(index) {
                skip_empty();
            }

            ArrowProxy operator->() const {
                auto& slot = (*m_slots)[m_index];
                auto entry = Entry(slot.key, slot.value);

                return ArrowProxy(entry);
            }

            Entry operator*() const {
                auto& slot = (*m_slots)[m_index];
                auto entry = Entry(slot.key, slot.value);

                return entry;
            }

            basic_iterator& operator++() {
                m_index++;
                skip_empty();
                return *this;
            }

            bool operator!=(const basic_iterator& other) const {
                return m_index != other.m_index;
            }

            bool operator==(const basic_iterator& other) const {
                return m_index == other.m_index;
            }

            operator basic_iterator<true>() const {
                return basic_iterator<true>(m_slots, m_index);
            }

        private:
            SlotPtr m_slots;
            size_t m_index;

            void skip_empty() {
                while (m_index < m_slots->size() && !(*m_slots)[m_index].occupied) {
                    m_index++;
                }
            }

            friend class basic_iterator<true>;
            friend class basic_iterator<false>;
        };

        std::vector<Slot> m_slots;
        std::size_t m_capacity;
        std::size_t m_size;
        Hasher m_hasher;

        size_t find_slot(const Key& key) {
            auto index = static_cast<size_t>(m_hasher(key)) % m_capacity;
            auto first_deleted = m_capacity;

            for (size_t probe = 0; probe < m_capacity; probe++) {
                auto& slot = m_slots[index];

                if (slot.occupied && slot.key == key) {
                    return index;
                }

                if (!slot.occupied) {
                    if (slot.deleted && first_deleted == m_capacity) {
                        first_deleted = index;
                    }
                    else if (!slot.deleted) {
                        return first_deleted != m_capacity ? first_deleted : index;
                    }
                }

                index = (index + 1) % m_capacity;
            }

            return first_deleted;
        }

        size_t find_slot_const(const Key& key) const {
            auto index = static_cast<size_t>(m_hasher(key)) % m_capacity;

            for (size_t probe = 0; probe < m_capacity; probe++) {
                auto& slot = m_slots[index];

                if (slot.occupied && slot.key == key) {
                    return index;
                }

                if (!slot.occupied && !slot.deleted) {
                    return m_capacity;
                }

                index = (index + 1) % m_capacity;
            }

            return m_capacity;
        }

        void grow() {
            auto old_slots = std::move(m_slots);

            m_capacity *= 2;
            m_size = 0;
            m_slots.clear();
            m_slots.resize(m_capacity);

            for (auto& slot : old_slots) {
                if (slot.occupied) {
                    insert(slot.key, std::move(slot.value));
                }
            }
        }

        size_t prepare_slot(const Key& key) {
            if (m_size * 4 >= m_capacity * 3) {
                grow();
            }

            auto index = find_slot(key);

            if (!m_slots[index].occupied) {
                m_size++;
            }

            m_slots[index].key = key;
            m_slots[index].occupied = true;
            m_slots[index].deleted = false;

            return index;
        }

    public:
        using iterator = basic_iterator<false>;
        using const_iterator = basic_iterator<true>;

        hash_map(size_t capacity = 16) {
            m_capacity = capacity;
            m_size = 0;
            m_slots.resize(capacity);
        }

        hash_map(std::initializer_list<std::pair<Key, Value>> entries) {
            m_capacity = entries.size() < 8 ? 16 : entries.size() * 2;
            m_size = 0;

            m_slots.resize(m_capacity);

            for (auto& entry : entries) {
                insert(entry.first, entry.second);
            }
        }

        void insert(const Key& key, const Value& value) {
            auto index = prepare_slot(key);

            m_slots[index].value = value;
        }

        void insert(const Key& key, Value&& value) {
            auto index = prepare_slot(key);

            m_slots[index].value = std::move(value);
        }

        bool contains(const Key& key) const {
            return find_slot_const(key) != m_capacity;
        }

        Value& at(const Key& key) {
            auto index = find_slot_const(key);

            if (index == m_capacity) {
                throw std::out_of_range("key not found in hash_map");
            }

            return m_slots[index].value;
        }

        const Value& at(const Key& key) const {
            auto index = find_slot_const(key);

            if (index == m_capacity) {
                throw std::out_of_range("key not found in hash_map");
            }

            return m_slots[index].value;
        }

        Value& operator[](const Key& key) {
            auto existing_index = find_slot_const(key);

            if (existing_index != m_capacity) {
                return m_slots[existing_index].value;
            }

            auto index = prepare_slot(key);

            return m_slots[index].value;
        }

        iterator find(const Key& key) {
            auto index = find_slot_const(key);

            if (index == m_capacity) {
                return end();
            }

            return iterator(&m_slots, index);
        }

        const_iterator find(const Key& key) const {
            auto index = find_slot_const(key);

            if (index == m_capacity) {
                return end();
            }

            return iterator(&m_slots, index);
        }

        void clear() {
            for (auto& slot : m_slots) {
                slot.occupied = false;
                slot.deleted = false;
            }

            m_size = 0;
        }

        void remove(const Key& key) {
            auto index = find_slot_const(key);

            if (index == m_capacity) {
                return;
            }

            m_slots[index].occupied = false;
            m_slots[index].deleted = true;
            m_size--;
        }

        size_t size() const {
            return m_size;
        }

        bool empty() const {
            return m_size == 0;
        }

        iterator begin() {
            return iterator(&m_slots, 0);
        }

        iterator end() {
            return iterator(&m_slots, m_slots.size());
        }

        const_iterator begin() const {
            return const_iterator(&m_slots, 0);
        }

        const_iterator end() const {
            return const_iterator(&m_slots, m_slots.size());
        }

        const_iterator cbegin() const {
            return const_iterator(&m_slots, 0);
        }

        const_iterator cend() const {
            return const_iterator(&m_slots, m_slots.size());
        }
    };
}

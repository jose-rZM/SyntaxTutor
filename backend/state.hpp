#pragma once
#include "lr0_item.hpp"
#include <algorithm>
#include <cstddef>
#include <functional>
#include <numeric>
#include <unordered_set>

struct state {
    std::unordered_set<Lr0Item> items_;
    unsigned int                id_;

    bool operator==(const state& other) const { return other.items_ == items_; }
};

namespace std {
template <> struct hash<state> {
    size_t operator()(const state& st) const {
        size_t seed =
            std::accumulate(st.items_.begin(), st.items_.end(), 0,
                            [](size_t acc, const Lr0Item& item) {
                                return acc ^ (std::hash<Lr0Item>()(item));
                            });
        return seed;
    }
};
} // namespace std

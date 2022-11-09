#pragma once

#include <Pacc/PaccPCH.hpp>

using String		= std::string;
using StringView	= std::string_view;

// Containers:
template <typename T, size_t N>
using Array = std::array<T, N>;

template <typename T>
using Vec = std::vector<T>;

template <typename Key, typename Value>
using Map = std::map<Key, Value>;

template <typename Key, typename Value>
using UMap = std::unordered_map<Key, Value>;

// Memory:
template <typename T>
using UPtr = std::unique_ptr<T>;

template <typename T>
using SPtr = std::shared_ptr<T>;

template <typename T>
using WPtr = std::weak_ptr<T>;

struct ReturnIdentity {
    template<typename U>
    constexpr auto operator()(U&& v) const noexcept
        -> decltype(std::forward<U>(v))
    {
        return std::forward<U>(v);
    }
};

template <typename T>
using Opt = std::optional<T>;

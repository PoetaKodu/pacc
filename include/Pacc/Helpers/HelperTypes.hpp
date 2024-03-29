#pragma once

#include <Pacc/PaccPCH.hpp>

using String		= std::string;
using StringView	= std::string_view;

using Path			= std::filesystem::path;

// Containers:
template <typename T, size_t N>
using Array = std::array<T, N>;

template <typename T, size_t Extent = std::dynamic_extent>
using Span = std::span<T, Extent>;

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

template <typename First, typename Second>
using Pair = std::pair<First, Second>;

template <typename T1, typename... Ts>
using Tuple = std::tuple<T1, Ts...>;

template <typename... Ts>
using Variant = std::variant<Ts...>;

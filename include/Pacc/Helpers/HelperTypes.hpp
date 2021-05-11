#pragma once

#include PACC_PCH

using VecOfStr 		= std::vector< std::string >;
using VecOfStrPtr 	= std::vector< std::string* >;


// Containers:
template <typename T>
using Vec = std::vector<T>;

template <typename Key, typename Value>
using Map = std::map<Key, Value>;

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
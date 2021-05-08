#pragma once

#include PACC_PCH

using VecOfStr 		= std::vector< std::string >;
using VecOfStrPtr 	= std::vector< std::string* >;


// Containers:
template <typename T>
using Vec = std::vector<T>;

// Memory:
template <typename T>
using UPtr = std::unique_ptr<T>;

template <typename T>
using SPtr = std::shared_ptr<T>;

template <typename T>
using WPtr = std::weak_ptr<T>;
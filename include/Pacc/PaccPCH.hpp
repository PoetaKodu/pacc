#pragma once

#ifndef PACC_PCH
#define PACC_PCH <Pacc/PaccPCH.hpp>
#endif

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <string_view>
#include <initializer_list>
#include <array>
#include <span>
#include <vector>
#include <tuple>
#include <stdexcept>
#include <optional>
#include <variant>
#include <thread>
#include <chrono>
#include <memory>
#include <filesystem>
#include <algorithm>
#include <cctype>
#include <queue>

#include <nlohmann/json.hpp>
#include <fmt/format.h>
#include <fmt/color.h>
#include <fmt/chrono.h>
#include <fmt/compile.h>
#include <fmt/args.h>

#include <TinyProcessLib/Process.hpp>

#define SOL_ALL_SAFETIES_ON 1

#include <sol/sol.hpp>

namespace fs 	= std::filesystem;
namespace rg	= std::ranges;
namespace ch 	= std::chrono;
namespace tt 	= std::this_thread;
namespace proc 	= TinyProcessLib;

using namespace nlohmann;

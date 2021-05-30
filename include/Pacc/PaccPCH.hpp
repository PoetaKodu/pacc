#pragma once

#ifndef PACC_PCH
#define PACC_PCH <Pacc/PaccPCH.hpp>
#endif

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <string_view>
#include <array>
#include <vector>
#include <tuple>
#include <stdexcept>
#include <optional>
#include <variant>
#include <thread>
#include <chrono>
#include <memory>
#include <filesystem>
#include <cctype>

#include <nlohmann/json.hpp>
#include <fmt/format.h>
#include <fmt/color.h>
#include <fmt/chrono.h>
#include <fmt/compile.h>

#include <TinyProcessLib/Process.hpp>

namespace fs 	= std::filesystem;
namespace ch 	= std::chrono;
namespace tt 	= std::this_thread;
namespace proc 	= TinyProcessLib;

using namespace nlohmann;
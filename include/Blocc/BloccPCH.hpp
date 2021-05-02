#pragma once

#ifndef BLOCC_PCH
#define BLOCC_PCH <Blocc/BloccPCH.hpp>
#endif

#include <iostream>
#include <fstream>
#include <string>
#include <string_view>
#include <array>
#include <vector>
#include <tuple>
#include <stdexcept>
#include <optional>
#include <thread>
#include <chrono>
#include <memory>
#include <filesystem>

#include <nlohmann/json.hpp>
#include <fmt/format.h>

#undef UNICODE
#include <tiny-process-lib/process.hpp>
#define UNICODE

namespace fs 	= std::filesystem;
namespace ch 	= std::chrono;
namespace tt 	= std::this_thread;
namespace proc 	= TinyProcessLib;

using namespace nlohmann;
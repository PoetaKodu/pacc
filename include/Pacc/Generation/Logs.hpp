#pragma once

#include PACC_PCH

#include <Pacc/Helpers/HelperTypes.hpp>

auto currentTimeForLog() -> String;

auto saveBuildOutputLog(StringView packageName_, String const& outputLog_) -> fs::path;

auto getSortedBuildLogs(size_t limit = 0) -> Vec<fs::path>;

#pragma once

#include PACC_PCH

std::string currentTimeForLog();

fs::path saveBuildOutputLog(std::string_view packageName_, std::string const& outputLog_);

std::vector<fs::path> getSortedBuildLogs(size_t limit = 0);
#include "include/Pacc/PaccPCH.hpp"

#include <Pacc/Generation/Logs.hpp>
#include <Pacc/System/Environment.hpp>

////////////////////////////////////////////
fs::path requireBuildLogsFolder()
{
	fs::path p = env::requirePaccDataStorageFolder() / "build_logs";
	fs::create_directories(p);
	return p;
}

////////////////////////////////////////////
fs::path saveBuildOutputLog(StringView packageName_, String const& outputLog_)
{

	fs::path p = requireBuildLogsFolder();

	p /= fmt::format(FMT_COMPILE("{}_{}.log"), currentTimeForLog(), packageName_);

	std::ofstream{ p } << outputLog_;
	return p;
}

////////////////////////////////////////////
auto getSortedBuildLogs(size_t limit) -> Vec<fs::path>
{
	auto logsPath = requireBuildLogsFolder();

	auto logs = Vec<fs::path>();

	logs.reserve(1024);

	for(auto it : fs::directory_iterator(logsPath))
	{
		if (it.is_regular_file())
		{
			fs::path p = it.path();
			if (p.extension() == ".log")
				logs.push_back(p);
		}
	}

	// Sort:
	rg::sort(logs, {}, &fs::path::filename);

	if (limit != 0 && logs.size() > 0)
		logs.resize( std::min(limit, logs.size()) );

	return logs;
}

////////////////////////////////////////////
auto currentTimeForLog() -> String
{
	return fmt::format("{:%Y_%m_%d_%H_%M_%S}", fmt::localtime( std::time(nullptr) ));
}

#include PACC_PCH

#include <Pacc/System/Environment.hpp>
#include <Pacc/System/Process.hpp>

namespace env
{

///////////////////////////////////////////////////
fs::path getPaccDataStorageFolder()
{
	// NOTE:
	// - on Windows folder is named `pacc`
	// - on Linux: `.pacc` -> with a preceding dot

	fs::path appData;
	#ifdef PACC_SYSTEM_WINDOWS
		appData = std::getenv("APPDATA");
		appData /= "pacc";
	#else
		appData = std::getenv("HOME");
		appData /= ".pacc";
	#endif
	
	return appData;
}

///////////////////////////////////////////////////
fs::path requirePaccDataStorageFolder()
{
	fs::path const storage = getPaccDataStorageFolder();
	fs::create_directories(storage);
	return storage;
}

///////////////////////////////////////////////////
fs::path findExecutable(std::string_view execName_)
{
	const std::string command = fmt::format(
		#ifdef PACC_SYSTEM_WINDOWS
			"where \"{}\"",
		#else
			"which \"{}\"",
		#endif
			execName_
		);

	// TODO:
	ChildProcess finder{command, "", ch::milliseconds{2500}};
	auto exitStatus = finder.runSync();

	if (exitStatus.value_or(1) == 0)
	{
		std::string& stdOut = finder.out.stdOut;

		// Parse `where execName` output
		size_t newLinePos = finder.out.stdOut.find_first_of("\r\n");
		if (newLinePos != std::string::npos)
			return stdOut.substr(0, newLinePos);
		else
			return stdOut;
	}
	
	return {};
}

}
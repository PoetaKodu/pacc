#include PACC_PCH

#include <Pacc/System/Environment.hpp>
#include <Pacc/System/Process.hpp>

#ifdef PACC_SYSTEM_WINDOWS
#define NOMINMAX
	#include <Windows.h>
#elif defined(PACC_SYSTEM_LINUX)
	#include <unistd.h>
#endif

namespace env
{

///////////////////////////////////////////////////
fs::path getPaccDataStorageFolder()
{
	// NOTE:
	// - on Windows folder is named `pacc`
	// - on Linux: `.pacc` -> with a preceding dot

	auto appData = fs::path();
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
	auto storage = getPaccDataStorageFolder();
	fs::create_directories(storage);
	return storage;
}

///////////////////////////////////////////////////
fs::path findExecutable(StringView execName_)
{
	const auto command = fmt::format(
		#ifdef PACC_SYSTEM_WINDOWS
			"where \"{}\"",
		#else
			"which \"{}\"",
		#endif
			execName_
		);

	// TODO:
	auto finder = ChildProcess{command, "", ch::milliseconds{2500}};
	auto exitStatus = finder.runSync();

	if (exitStatus.value_or(1) == 0)
	{
		auto& stdOut = finder.out.stdOut;

		// Parse `where execName` output
		auto newLinePos = finder.out.stdOut.find_first_of("\r\n");
		if (newLinePos != String::npos)
			return stdOut.substr(0, newLinePos);
		else
			return stdOut;
	}

	return {};
}

///////////////////////////////////////////////////
fs::path getPaccAppPath()
{
	std::array<char, 4 * 1024> buf;

	#ifdef PACC_SYSTEM_WINDOWS
		size_t bytes;
	#elif defined(PACC_SYSTEM_LINUX)
		ssize_t bytes;
	#endif

	// Obtain the path in `buf` and length in `bytes`
	#ifdef PACC_SYSTEM_WINDOWS
		bytes = static_cast<std::size_t>( GetModuleFileNameA(nullptr, buf.data(), static_cast<DWORD>(buf.size()) ) );
	#elif defined(PACC_SYSTEM_LINUX)
		bytes = std::min(readlink("/proc/self/exe", buf.data(), buf.size()), ssize_t(buf.size() - 1));
	#endif

	return fs::path(String(buf.data(), bytes));
}



}

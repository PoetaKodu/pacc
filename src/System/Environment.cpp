#include PACC_PCH

#include <Pacc/System/Environment.hpp>

namespace env
{

///////////////////////////////////////////////////
fs::path getPaccDataStorageFolder()
{
	fs::path appData;
	#ifdef PACC_SYSTEM_WINDOWS
		appData = std::getenv("APPDATA");
	#else
		appData = std::getenv("USER");
	#endif
	appData /= "pacc";
	return appData;
}

///////////////////////////////////////////////////
fs::path requirePaccDataStorageFolder()
{
	fs::path const storage = getPaccDataStorageFolder();
	fs::create_directories(storage);
	return storage;
}

}
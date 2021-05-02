#include BLOCC_PCH

namespace env
{

///////////////////////////////////////////////////
fs::path getBloccDataStorageFolder()
{
	fs::path appData;
	#ifdef BLOCC_SYSTEM_WINDOWS
		appData = std::getenv("APPDATA");
	#else
		appData = std::getenv("USER");
	#endif
	appData /= "blocc";
	return appData;
}

///////////////////////////////////////////////////
fs::path requireBloccDataStorageFolder()
{
	fs::path const storage = getBloccDataStorageFolder();
	fs::create_directories(storage);
	return storage;
}

}
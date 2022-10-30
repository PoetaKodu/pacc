#include "include/Pacc/PaccPCH.hpp"

#include <Pacc/App/App.hpp>

#include <Pacc/System/Environment.hpp>

//////////////////////////////////////
sol::protected_function_result PaccApp::execLuaEvent(Package& pkg_, String const& eventName_)
{
	auto it = pkg_.scripts.find(eventName_);

	if (it != pkg_.scripts.end()) {
		auto& funcName = it->second->functionName;
		auto luaFn = lua[funcName];
		if (luaFn.get_type() != sol::type::function) {
			throw PaccException("Could not execute event \"{}\"!\nFunction \"{}\" not defined within the specified module.", eventName_, funcName);
		}

		auto result = luaFn(std::ref(pkg_));

		if (!result.valid()) {
			throw PaccException("Could not execute event \"{}\"!\nDetails: {}", eventName_, result.get<sol::error>().what());
		}
		return result;
	}

	return {};
}

////////////////////////////////////
// Enables view of the underlying container of the priority_queue
template <class T, class S, class C>
S const& viewContainer(std::priority_queue<T, S, C> const& q)
{
    struct ViewableQueue : private std::priority_queue<T, S, C> {
        static S const& viewContainer(std::priority_queue<T, S, C> const& q) {
            return q.*&ViewableQueue::c;
        }
    };
    return ViewableQueue::viewContainer(q);
}

//////////////////////////////////////
auto PaccApp::detectPreferredPackageLoaderFor(fs::path const& path_) const
	-> IPackageLoader&
{
	if (defaultPackageLoader->canLoad(path_)) {
		return *defaultPackageLoader;
	}
	else {
		for (auto loader : viewContainer(autodetectPackageLoaders))
		{
			if (loader->canLoad(path_)) {
				return *loader;
			}
		}
	}

	throw PaccException("Could not auto-detect package loader for \"{}\"!", path_.string())
		.withHelp("Please specify the package loader explicitly or provide a pacc.json file or other supported package format.");
}

//////////////////////////////////////
auto PaccApp::loadPackage(fs::path const& path_)
	-> UPtr<Package>
{
	return defaultPackageLoader->load(path_);
}

//////////////////////////////////////
auto PaccApp::loadPackage(fs::path const& path_, String const& loaderName_)
	-> UPtr<Package>
{
	if (loaderName_ == "auto") {
		return this->detectPreferredPackageLoaderFor(path_).load(path_);
	}
	else {
		auto it = packageLoaders.find(loaderName_);
		if (it == packageLoaders.end()) {
			throw PaccException("Could not load package \"{}\"!\nPackage loader \"{}\" not found.", path_.string(), loaderName_);
		}

		return it->second->load(path_);
	}
}

//////////////////////////////////////
auto PaccApp::loadPackageByName(
		String const&		name_,
		VersionRequirement	verReq_,
		UPtr<Package>*		invalidVersion_,
		String const&		loaderName_
	)
	-> UPtr<Package>
{
	auto candidates = Vec<fs::path>{
			fs::current_path() 					/ "pacc_packages",
			// Folder above that is inside pacc_packages folder
			fs::current_path() 					/ "../../pacc_packages",
			env::getPaccDataStorageFolder() 	/ "packages"
		};

	// Get first matching candidate:
	for(auto const& c : candidates)
	{
		auto pkgFolder = c / name_;
		UPtr<Package> pkg;
		try {
			pkg = this->loadPackage(pkgFolder, loaderName_);
		}
		catch(...) {
			// Could not load, ignore
			continue;
		}

		if (verReq_.test(pkg->version))
			return pkg;
		else
		{
			if (invalidVersion_)
				*invalidVersion_ = std::move(pkg);
		}
	}

	// (TODO: help here)
	// Found none.
	throw PaccException("Could not find package \"{}\".", name_);

}

#include "include/Pacc/PaccPCH.hpp"

#include <Pacc/App/App.hpp>

#include <Pacc/System/Environment.hpp>

////////////////////////////////////
// Enables view of the underlying container of the priority_queue
template <class T, class Cont, class P>
auto viewContainer(std::priority_queue<T, Cont, P> const& q) -> Cont const&
{
	struct ViewableQueue
		: private std::priority_queue<T, Cont, P>
	{
		static auto viewContainer(std::priority_queue<T, Cont, P> const& q) -> Cont const&
		{
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
		auto pkg = UPtr<Package>();
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

#include "include/Pacc/PaccPCH.hpp"

#include <Pacc/App/App.hpp>
#include <Pacc/Helpers/Lua.hpp>
#include <Pacc/PackageSystem/MainPackageLoader.hpp>

/////////////////////////////////////////////
auto MainPackageLoader::load(fs::path const& root_) -> UPtr<Package>
{
	auto preloaded	= Package::preload(root_);
	auto usesScript	= preloaded.usesScriptFile();

	if (usesScript)
	{
		auto script = app.lua.load_file(preloaded.scriptFile.string());
		if (!script.valid())
		{
			throw PaccException("{}", getError(script).what());
		}

		auto result = script();

		if (!result.valid())
		{
			throw PaccException("{}", getError(result).what());
		}
	}

	auto pkg = Package::load( std::move(preloaded) );

	if (usesScript)
	{
		app.execPackageEvent(*pkg, "onPostLoad");
	}

	return pkg;
}

/////////////////////////////////////////////
auto MainPackageLoader::canLoad(fs::path const& root_) const -> bool
{
	return fs::is_directory(root_) && !findPackageFile(root_).empty();
}


/////////////////////////////////////////////
auto MainPackageLoader::loadTarget(fs::path const& root_, String const& name_, TargetBase& target_) -> bool
{
	auto pkg = load(root_);

	auto it = rg::find(pkg->projects, name_, &Project::name);

	if (it == pkg->projects.end())
		return false;

	target_ = *it;
	return true;
}

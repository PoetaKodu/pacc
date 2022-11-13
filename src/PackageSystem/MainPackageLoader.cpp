#include "include/Pacc/PaccPCH.hpp"

#include <Pacc/App/App.hpp>
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
			auto err = sol::error(script);
			throw PaccException("{}", err.what());
		}

		auto result = script();

		if (!result.valid())
		{
			auto err = sol::error(result);
			throw PaccException("{}", err.what());
		}
	}

	auto pkg = Package::load( std::move(preloaded) );

	if (usesScript)
	{
		// lua["rootPackage"] = std::ref(*pkg);

		// disp(lua["pacc"], "pre:generate");
		// disp(lua["pacc"], "build", std::ref(pkg->projects[0]));

		app.execLuaEvent(*pkg, "onPostLoad");
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

	auto it = std::find_if(pkg->projects.begin(), pkg->projects.end(),
		[&name_](Project const& p)
		{
			return p.name == name_;
		});

	if (it == pkg->projects.end())
		return false;

	target_ = *it;
	return true;
}

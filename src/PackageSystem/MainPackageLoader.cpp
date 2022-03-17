#include PACC_PCH

#include <Pacc/App/App.hpp>
#include <Pacc/PackageSystem/MainPackageLoader.hpp>

/////////////////////////////////////////////
UPtr<Package> MainPackageLoader::load(fs::path const& root_)
{
	auto preloaded	= Package::preload(root_);
	bool usesScript	= preloaded.usesScriptFile();

	if (usesScript)
	{
		auto script = app.lua.load_file(preloaded.scriptFile.string());
		if (!script.valid())
		{
			sol::error err = script;
			throw PaccException("{}", err.what());
		}

		auto result = script();

		if (!result.valid())
		{
			sol::error err = result;
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
bool MainPackageLoader::canLoad(fs::path const& root_) const
{
	return fs::is_directory(root_) && fs::is_regular_file(root_ / "cpackage.json");
}


/////////////////////////////////////////////
bool MainPackageLoader::loadTarget(fs::path const& root_, std::string const& name_, TargetBase& target_)
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

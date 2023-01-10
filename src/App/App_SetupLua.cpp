#include "include/Pacc/PaccPCH.hpp"

#include <Pacc/App/App.hpp>
#include <Pacc/PackageSystem/Version.hpp>
#include <Pacc/System/Environment.hpp>
#include <Pacc/System/Filesystem.hpp>
#include <Pacc/Helpers/Lua.hpp>

auto loadPaccLuaSDK(sol::state& lua, Path const& searchPattern) -> void
{
	auto currentPackagesPath = lua["package"]["path"].get<String>();

	lua["package"]["path"].set(currentPackagesPath + ";" + searchPattern.string());

	lua.script("require(\"pacc-sdk/app\")");

	if (lua["pacc"].get_or<sol::object>(sol::nil) == sol::nil)
	{
		throw PaccException(
			"pacc attempted to load bundled lua SDK, but it failed. Reason: \"pacc-sdk/app.lua\" submodule didn't provide the \"pacc\" object.\n"
			"This could be caused by a missing lua SDK, or by a broken lua SDK or moving the pacc executable to the wrong directory."
		).withHelp(
			"Ensure that the lua SDK is located in: \"{}\"\n"
			"... or specify custom SDK path using the \"--lua-lib\" command line argument.",
			searchPattern.parent_path().string()
		).withNote(
			"Previous pacc versions used the name \"pacc-std\" instead of \"pacc-sdk\". Make sure you use the correct pacc-sdk version."
		);
	}

	lua["pacc"]["version"] = Version::fromString("0.6.1");
}

auto PaccApp::setupLua() -> void
{
	lua = freshLuaInstance();

	// Insert the pacc lua SDK
	{
		auto paccLuaSDKSearch = Path();

		// Default value: the `lua` folder is a sibling of the `bin`, where the pacc executable is located.
		auto& flag = *settings.flags.at("--lua-lib");
		if (!flag.isSet())
		{
			paccLuaSDKSearch = env::getPaccAppPath().parent_path() / "../lua/?.lua";
		}
		else // User specified value
		{
			paccLuaSDKSearch = Path(flag.value);
			if (!paccLuaSDKSearch.is_absolute())
			{
				paccLuaSDKSearch = initialWorkingDirectory / paccLuaSDKSearch;
			}

			auto patternIsPresent = flag.value.ends_with("?.lua");
			if (!patternIsPresent)
			{
				// Note: we're using "/=" here instead of "+=" operator
				// because user could specify a path without a trailing slash
				paccLuaSDKSearch /= "?.lua";
			}
		}

		loadPaccLuaSDK(lua, paccLuaSDKSearch);
	}
}

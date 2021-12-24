#include PACC_PCH

#include <Pacc/App/App.hpp>
#include <Pacc/PackageSystem/Version.hpp>

//////////////////////////////////////////////////
auto loadFmtArgs(sol::variadic_args va)
{
	auto store = fmt::dynamic_format_arg_store<fmt::format_context>();

	for (auto arg : va)
	{
		if (arg.is<std::string>())
			store.push_back(arg.as<std::string>());
		else if (arg.is<int>())
			store.push_back(arg.as<int>());
		else if (arg.is<double>())
			store.push_back(arg.as<double>());
		else if (arg.is<bool>())
			store.push_back(arg.as<bool>());
	}
	return store;
}

//////////////////////////////////////////////////
void PaccApp::setupLua()
{
	lua["fmt"] = lua.create_table_with(
		"string",
		[](std::string fmt_, sol::variadic_args va) -> std::string
		{
			return fmt::vformat(fmt_, loadFmtArgs(va));
		},
		"print",
		[](std::string fmt_, sol::variadic_args va)
		{
			fmt::vprint(fmt_, loadFmtArgs(va));
		}
	);

	lua.new_usertype<Version>("Version",
			"major",	&Version::major,
			"minor",	&Version::minor,
			"patch",	&Version::patch,
			"toString",	&Version::toString
		);

	lua.new_usertype<Project>("Project",
			"name",		&Project::name
		);

	lua.new_usertype<Package>("CPackage",
			"name",		&Package::name,
			"root",		[] (Package const& pkg) { return pkg.root.string(); },
			"version",	&Package::version,
			"projects",	&Package::projects
		);

	lua.script("require(\"pacc-std/app\")");

	lua["pacc"]["version"] = Version::fromString("0.3.2");
}

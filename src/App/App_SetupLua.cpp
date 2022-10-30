#include "include/Pacc/PaccPCH.hpp"

#include <Pacc/App/App.hpp>
#include <Pacc/PackageSystem/Version.hpp>
#include <Pacc/System/Environment.hpp>

//////////////////////////////////////////////////
auto loadFmtArgs(sol::variadic_args va)
{
	auto store = fmt::dynamic_format_arg_store<fmt::format_context>();

	for (auto arg : va)
	{
		if (arg.is<String>())
			store.push_back(arg.as<String>());
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
	using namespace sol;

	lua.open_libraries(
			lib::base,
			lib::package,
			lib::table,
			lib::string
		);

	lua["fmt"] = lua.create_table_with(
		"string",
		[](String fmt_, sol::variadic_args va) -> String
		{
			return fmt::vformat(fmt_, loadFmtArgs(va));
		},
		"print",
		[](String fmt_, sol::variadic_args va)
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

	struct AccessWrapperGetter {
		AccessSplitVec<String> const* ptr;
		auto public_() const { return std::ref(ptr->public_); }
		auto private_() const { return std::ref(ptr->private_); }
		auto protected_() const { return std::ref(ptr->interface_); }
	};
	struct AccessWrapperSetter {
		AccessSplitVec<String>* ptr;
		auto public_() const { return std::ref(ptr->public_); }
		auto private_() const { return std::ref(ptr->private_); }
		auto protected_() const { return std::ref(ptr->interface_); }
		auto public_() { return std::ref(ptr->public_); }
		auto private_() { return std::ref(ptr->private_); }
		auto protected_() { return std::ref(ptr->interface_); }
	};

	lua.new_usertype<AccessWrapperGetter>("AccessWrapperGetter",
			"public", [] (AccessWrapperGetter const& w) { return w.public_(); },
			"private", [] (AccessWrapperGetter const& w) { return w.private_(); },
			"protected", [] (AccessWrapperGetter const& w) { return w.protected_(); }
		);

	lua.new_usertype<AccessWrapperSetter>("AccessWrapperSetter",
			"public", sol::property([] (AccessWrapperSetter const& w) { return w.public_(); }, [] (AccessWrapperSetter const& w) { return w.public_(); } ),
			"private", sol::property([] (AccessWrapperSetter const& w) { return w.private_(); }, [] (AccessWrapperSetter const& w) { return w.private_(); } ),
			"protected", sol::property([] (AccessWrapperSetter const& w) { return w.protected_(); }, [] (AccessWrapperSetter const& w) { return w.protected_(); } )
		);

	lua.new_usertype<Project>("Project",
			"name",		&Project::name,
			"kind",		sol::property(
				[](Project const& proj) { return toString(proj.type); },
				[](Project& proj, String const& val_) { proj.type = parseProjectType(val_); }
			),
			"includeFolders", sol::property(
				[](Project const& proj) { return AccessWrapperGetter{&proj.includeFolders.self}; },
				[](Project& proj) { return AccessWrapperSetter{&proj.includeFolders.self}; }
			),
			"files", sol::property(
				[](Project const& proj) { return std::ref(proj.files); },
				[](Project& proj) { return std::ref(proj.files); }
			)
		);

	lua.new_usertype<Package>("CPackage",
			"name",		&Package::name,
			"root",		[] (Package const& pkg) { return pkg.root.string(); },
			"version",	&Package::version,
			"projects",	&Package::projects,
			"addProject", [](Package& pkg, String name)
			{
				Project p;
				p.name = std::move(name);
				pkg.projects.emplace_back(std::move(p));
				return std::ref(pkg.projects.back());
			}
		);


	auto currentPackagesPath	= lua["package"]["path"].get<String>();
	auto paccLuaScriptsPattern	= env::getPaccAppPath().parent_path() / "../lua/?.lua";

	lua["package"]["path"].set(currentPackagesPath + ";" + paccLuaScriptsPattern.string());
	lua.script("require(\"pacc-std/app\")");

	lua["pacc"]["version"] = Version::fromString("0.5.0");
}

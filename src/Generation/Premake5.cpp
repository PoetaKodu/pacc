#include "include/Pacc/PaccPCH.hpp"

#include <Pacc/App/App.hpp>
#include <Pacc/Generation/Premake5.hpp>
#include <Pacc/Generation/OutputFormatter.hpp>
#include <Pacc/System/Filesystem.hpp>
#include <Pacc/System/Environment.hpp>
#include <Pacc/System/Process.hpp>
#include <Pacc/Helpers/Exceptions.hpp>
#include <Pacc/Helpers/String.hpp>

using namespace fmt;

namespace constants
{

/////////////////////////////////////////////////////////////////////
constexpr StringView DefaultPremakeCfg =
R"DefaultCfg(
platforms { "x86", "x64" }
	configurations { "Debug", "Release" }

	location ("build")
	targetdir(path.join(os.getcwd(), "bin/%{cfg.platform}/%{cfg.buildcfg}"))

	if os.host() == "macosx" then
		removeplatforms { "x86" }
	end

	filter "platforms:*32"
		architecture "x86"

	filter "platforms:*64"
		architecture "x86_64"

	filter "configurations:Debug"
		defines { "DEBUG" }
		symbols "On"

	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"

	filter {}
)DefaultCfg";

namespace mappings
{

/////////////////////////////////////////////////////////////////////
auto const& LangToPremakeLangAndDialect()
{
	using LangAndDialect = std::pair<StringView, StringView>;
	static const std::map<StringView, LangAndDialect, IgnoreCaseLess> dict =
		{
			{ "C89", 	{ "C", 		"" } },
			{ "C90", 	{ "C", 		"" } },
			{ "C95", 	{ "C", 		"" } },
			{ "C99", 	{ "C", 		"" } },
			{ "C11", 	{ "C", 		"" } },
			{ "C17", 	{ "C", 		"" } },
			{ "C++98", 	{ "C++", 	"C++98" } },
			{ "C++0x", 	{ "C++", 	"C++11" } },
			{ "C++11", 	{ "C++", 	"C++11" } },
			{ "C++1y", 	{ "C++", 	"C++14" } },
			{ "C++14", 	{ "C++", 	"C++14" } },
			{ "C++1z", 	{ "C++", 	"C++17" } },
			{ "C++17", 	{ "C++", 	"C++17" } },
			{ "C++20", 	{ "C++", 	"C++20" } },
		};
	return dict;
}

/////////////////////////////////////////////////////////////////////
auto const& AppTypeToPremakeKind()
{
	static const std::map<Project::Type, StringView> AppTypeToPremakeKind =
		{
			{ Project::Type::App, 		"ConsoleApp" },
			{ Project::Type::StaticLib, "StaticLib" },
			{ Project::Type::SharedLib, "SharedLib" }
		};
	return AppTypeToPremakeKind;
}

}

}


namespace gen
{

/////////////////////////////////////////////////
// Helper functions and forward declarations:


void appendWorkspace		(OutputFormatter &fmt_, Package const& pkg_);
void appendProject			(OutputFormatter &fmt_, Package const& pkg_, Project const& project_);
void appendConfiguration	(OutputFormatter &fmt_, Package const& pkg_, Project const& project_, Configuration const& config_);
template <typename T>
void appendPropWithAccess	(OutputFormatter &fmt_, StringView propName, T const& values_, MultiAccess accesses_ = MultiAccess::NoInterface);
template <typename T>
void appendStringsWithAccess(OutputFormatter &fmt_, T const& vec_, MultiAccess accesses_ = MultiAccess::NoInterface);
void appendStrings(OutputFormatter &fmt_, Vec<String> const& vec_);

/////////////////////////////////////////////////
void Premake5::generate(Package const & pkg_)
{
	// Prepare output buffer
	String out;
	out.reserve(4 * 1024 * 1024);

	auto fmt = OutputFormatter{out};

	appendWorkspace(fmt, pkg_);

	// Store the output in the premake file
	std::ofstream("premake5.lua") << out;

	if (compileCommands)
	{
		try {
			exportCompileCommands();
		}
		catch(PaccException&) {
			// Ignore.
		}
	}
}

/////////////////////////////////////////////////
bool Premake5::exportCompileCommands()
{
	auto& app = useApp();
	auto premake5Path = app.getPremake5Path();

	auto premake5ScriptPath = Path();

	{
		auto& luaLibFlag = *app.settings.flags.at("--lua-lib");
		if (luaLibFlag.isSet())
		{
			premake5ScriptPath = Path(luaLibFlag.value) / "premake";
		}
		else
		{
			// Example:
			// Premake5 path:			C:\Program Files\pacc\bin\premake5.exe
			// Premake scripts library:	C:\Program Files\pacc\lua\premake
			premake5ScriptPath = premake5Path.parent_path().parent_path() / "lua/premake";
		}
	}

	using fmt::fg, fmt::color;

	fmt::print(fg(color::gray), "Exporting compile commands... ");

	auto command = fmt::format("\"{}\" \"--scripts={}\" export-compile-commands", premake5Path.string(), premake5ScriptPath.string());
	auto exitStatus = ChildProcess{command, "", ch::seconds{30}}.runSync();

	if (exitStatus.has_value())
	{
		if (exitStatus.value() == 0)
		{
			fmt::print(fg(color::green), "success (build/compile_commands/*.json)\n");
			return true;
		}

		fmt::printErr(fg(color::red), "failure ({})\n", exitStatus.value());
		return false;
	}

	fmt::printErr(fg(color::red), "timeout\n");
	return false;
}

/////////////////////////////////////////////////
void appendWorkspace(OutputFormatter &fmt_, Package const& pkg_)
{
	fmt_.write("workspace(\"{}\")\n", pkg_.name);

	{
		auto indent = IndentScope{fmt_};

		// TODO: manual configuration
		fmt_.writeRaw(constants::DefaultPremakeCfg);

		for(auto const& project : pkg_.projects)
		{
			if (project.type != Project::Type::Interface)
				appendProject(fmt_, pkg_, project);
		}
	}
}




/////////////////////////////////////////////////
StringView mapToPremake5Kind(Project::Type projectType_)
{
	auto const& AppTypeMapping = constants::mappings::AppTypeToPremakeKind();

	auto it = AppTypeMapping.find(projectType_);
	if (it != AppTypeMapping.end())
		return it->second;

	return "";
}

/////////////////////////////////////////////////
void appendPremake5Lang(OutputFormatter& fmt_, StringView lang_)
{
	auto const& LangMapping = constants::mappings::LangToPremakeLangAndDialect();

	auto it = LangMapping.find(lang_);
	if (it != LangMapping.end())
	{
		auto const& premakeVal = it->second;

		fmt_.write("language (\"{}\")\n", premakeVal.first);
		if (!premakeVal.second.empty())
			fmt_.write("cppdialect (\"{}\")\n", premakeVal.second);
	}
}

/////////////////////////////////////////////////
void appendProject(OutputFormatter &fmt_, Package const& pkg_, Project const& project_)
{
	fmt_.write("\n");
	fmt_.write("project(\"{}\")\n", project_.name);

	// Format project settings:
	{
		auto indent = IndentScope{fmt_};

		// TODO: value mapping (enums, etc)
		fmt_.write("kind(\"{}\")\n", mapToPremake5Kind(project_.type));

		// TODO: extract this to functions
		// TODO: merge language and c/cpp dialect into one
		if (!project_.language.empty())
			appendPremake5Lang(fmt_, project_.language);
		else {
			// TODO: use configuration file to get default values
			fmt_.write("language(\"C++\")\n");
			fmt_.write("cppdialect(\"C++17\")\n");
		}

		if (project_.pch.has_value())
		{
			auto const& pch = project_.pch.value();
			fmt_.write("-- Precompiled header:\n");

			fmt_.write("pchheader(\"{}\")\n", pch.header);
			fmt_.write("pchsource(\"{}\")\n", pch.source);

			fmt_.write("defines ( {{ \"{}=\\\"{}\\\"\" }} )\n", pch.definition, pch.header);

			fmt_.write("includedirs( {{ \".\" }} )\n\n");
		}

		auto& app = useApp();
		if (app.settings.isFlagSet("--cores")) {
			auto cores = app.settings.tryGetFlagValue<int>("--cores");

			if (cores.value_or(1) > 1)
			{
				fmt_.write("flags ({{ \"MultiProcessorCompile\" }})\n");
			}
		}

		appendConfiguration(fmt_, pkg_, project_, project_);

		// TODO: keep insertion order
		for (auto filterIt : project_.premakeFilters)
		{
			fmt_.write("\n");
			fmt_.write("filter(\"{}\")\n", filterIt.first);
			{
				auto indent = IndentScope{fmt_};
				appendConfiguration(fmt_, pkg_, project_, filterIt.second);
				fmt_.write("filter(\"\")\n");
			}
		}
	}
}


/////////////////////////////////////////////////
void appendConfiguration(OutputFormatter &fmt_, Package const& pkg_, Project const& project_, Configuration const& config_)
{
	// TODO: Refactor this code

	if (!config_.moduleDefinitionFile.empty())
	{
		fmt_.write("-- Use .def file with Visual Studio compiler.\n");
		fmt_.write("if string.match(_ACTION, \"vs%d%d%d%d\") ~= nil then\n");
		{
			auto indent = IndentScope{fmt_};
			fmt_.write("linkoptions ({{ \"/DEF:\\\"{}\\\"\" }})\n", fsx::fwd(pkg_.root.parent_path() / config_.moduleDefinitionFile).string());
		}
		fmt_.write("end\n");
	}

	if (config_.symbolVisibility != GNUSymbolVisibility::Default)
	{
		fmt_.write("-- Symbol visibility (affects only GNU linker)");
		fmt_.write("visibility(\"{}\")\n", config_.symbolVisibility.toString());
	}

	auto computedLinkMode = MultiAccess::NoInterface;

	if (project_.isLibrary())
		computedLinkMode = MultiAccess::Private; // append only private deps

	// Computed first:
	appendPropWithAccess(fmt_, "defines", 		config_.defines.computed);
	appendPropWithAccess(fmt_, "links", 		config_.linkedLibraries.computed, computedLinkMode);
	appendPropWithAccess(fmt_, "includedirs", 	config_.includeFolders.computed);
	appendPropWithAccess(fmt_, "libdirs", 		config_.linkerFolders.computed, computedLinkMode);
	appendPropWithAccess(fmt_, "buildoptions", 	config_.compilerOptions.computed);
	appendPropWithAccess(fmt_, "linkoptions", 	config_.linkerOptions.computed);

	appendPropWithAccess(fmt_, "files", 		config_.files);
	appendPropWithAccess(fmt_, "defines", 		config_.defines.self);
	appendPropWithAccess(fmt_, "links", 		config_.linkedLibraries.self);
	appendPropWithAccess(fmt_, "includedirs", 	config_.includeFolders.self);
	appendPropWithAccess(fmt_, "libdirs", 		config_.linkerFolders.self);
	appendPropWithAccess(fmt_, "buildoptions", 	config_.compilerOptions.self);
	appendPropWithAccess(fmt_, "linkoptions", 	config_.linkerOptions.self);
}

/////////////////////////////////////////////////
template <typename T>
void appendPropWithAccess(OutputFormatter &fmt_, StringView propName, T const& values_, MultiAccess accesses_)
{
	if (getNumElements(values_) > 0)
	{
		fmt_.write("{} ({{\n", propName);
		{
			auto indent = IndentScope{fmt_};
			appendStringsWithAccess(fmt_, values_, accesses_);
		}
		fmt_.write("}})\n");
	}
}


/////////////////////////////////////////////////
template <typename T>
void appendStringsWithAccess(OutputFormatter &fmt_, T const& acc_, MultiAccess accesses_)
{
	for(auto const* acc : getAccesses(acc_, accesses_))
	{
		if (acc->size() > 0)
		{
			appendStrings(fmt_, *acc);
			fmt_.write("\n");
		}
	}
}

/////////////////////////////////////////////////
void appendStrings(OutputFormatter &fmt_, Vec<String> const& vec_)
{
	for(auto const & str : vec_)
		fmt_.write("\"{}\",\n", replaceAll(str, "\"", "\\\""));
}



}

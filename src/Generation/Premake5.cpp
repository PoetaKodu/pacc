#include PACC_PCH

#include <Pacc/Generation/Premake5.hpp>
#include <Pacc/Generation/OutputFormatter.hpp>
#include <Pacc/System/Filesystem.hpp>
#include <Pacc/System/Environment.hpp>
#include <Pacc/System/Process.hpp>
#include <Pacc/Helpers/Exceptions.hpp>
#include <Pacc/Helpers/String.hpp>

using namespace fmt;

template <typename T = std::string_view>
using DictElem 	= std::pair<std::string_view, T>;
template <typename T = std::string_view>
using Dict 		= std::vector< DictElem<T> >;

namespace constants
{

/////////////////////////////////////////////////////////////////////
constexpr std::string_view DefaultPremakeCfg =
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
	using LangAndDialect = std::pair<std::string_view, std::string_view>;
	static const Dict<LangAndDialect> dict = {
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
		{ "C++17", 	{ "C++", 	"C++17" } }
	};
	return dict;
}

/////////////////////////////////////////////////////////////////////
auto const& AppTypeToPremakeKind()
{
	static const Dict<> AppTypeToPremakeKind = {
		{ "app", 		"ConsoleApp" },
		{ "static lib", "StaticLib" },
		{ "shared lib", "SharedLib" }
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
void appendPropWithAccess	(OutputFormatter &fmt_, std::string_view propName, T const& values_);
template <typename T>
void appendStringsWithAccess(OutputFormatter &fmt_, T const& vec_);

/////////////////////////////////////////////////
template <typename T>
typename Dict<T>::const_iterator mapString(Dict<T> const& dict_, std::string_view v);

/////////////////////////////////////////////////
void runPremakeGeneration(std::string_view toolchainName_)
{
	using fmt::fg, fmt::color;

	fmt::print(fg(color::gray), "Running Premake5... ");

	std::string command = "premake5 ";
	command += toolchainName_;
	
	auto exitStatus = ChildProcess{command, "", ch::seconds{10}}.runSync();

	if (exitStatus.has_value())
	{
		if (exitStatus.value() == 0)
			fmt::print(fg(color::green), "success\n");
	}
	else
		fmt::printErr(fg(color::red), "timeout\n");

	if (int es = exitStatus.value_or(1))
	{
		throw PaccException("Failed to generate project files (Premake5 exit code: {})", es);
	}
}


/////////////////////////////////////////////////
void Premake5::generate(Package & pkg_, BuildQueueBuilder & depQueue)
{
	// Prepare output buffer
	std::string out;
	out.reserve(4 * 1024 * 1024);

	OutputFormatter fmt{out};

	// TODO: make it a single-step setup:
	depQueue.performConfigurationMerging();


	appendWorkspace(fmt, pkg_);

	// Store the output in the premake file
	std::ofstream("premake5.lua") << out;
}


/////////////////////////////////////////////////
void appendWorkspace(OutputFormatter &fmt_, Package const& pkg_)
{
	fmt_.write("workspace(\"{}\")\n", pkg_.name);

	{
		IndentScope indent{fmt_};

		// TODO: manual configuration
		fmt_.writeRaw(constants::DefaultPremakeCfg);
		
		for(auto const& project : pkg_.projects)
		{
			if (project.type != "interface")
				appendProject(fmt_, pkg_, project);
		}
	}
}




/////////////////////////////////////////////////
std::string_view mapToPremake5Kind(std::string_view projectType_)
{
	auto const& AppTypeMapping = constants::mappings::AppTypeToPremakeKind();

	auto it = mapString(AppTypeMapping, projectType_);
	if (it != AppTypeMapping.end())
		return it->second;
	
	return "";
}

/////////////////////////////////////////////////
void appendPremake5Lang(OutputFormatter& fmt_, std::string_view lang_)
{
	auto const& LangMapping = constants::mappings::LangToPremakeLangAndDialect();

	auto it = mapString(LangMapping, lang_);
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
		IndentScope indent{fmt_};

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
			PrecompiledHeader const& pch = project_.pch.value();
			fmt_.write("-- Precompiled header:\n");

			fmt_.write("pchheader(\"{}\")\n", pch.header);
			fmt_.write("pchsource(\"{}\")\n", pch.source);
			fmt_.write("defines ( {{ \"{}=\\\"{}\\\"\" }} )\n", pch.definition, pch.header);
			fmt_.write("includedirs( {{ \".\" }} )\n\n");
		}

		appendConfiguration(fmt_, pkg_, project_, project_);

		// TODO: keep insertion order
		for (auto filterIt : project_.premakeFilters)
		{
			fmt_.write("\n");
			fmt_.write("filter(\"{}\")\n", filterIt.first);
			{
				IndentScope indent{fmt_};
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
			IndentScope indent{fmt_};
			fmt_.write("linkoptions ({{ \"/DEF:\\\"{}\\\"\" }})\n", fsx::fwd(pkg_.root.parent_path() / config_.moduleDefinitionFile).string());
		}
		fmt_.write("end\n");
	}

	// Computed first:
	appendPropWithAccess(fmt_, "defines", 		config_.defines.computed);
	appendPropWithAccess(fmt_, "links", 		config_.linkedLibraries.computed);
	appendPropWithAccess(fmt_, "includedirs", 	config_.includeFolders.computed);
	appendPropWithAccess(fmt_, "libdirs", 		config_.linkerFolders.computed);
	
	appendPropWithAccess(fmt_, "files", 		config_.files);
	appendPropWithAccess(fmt_, "defines", 		config_.defines.self);
	appendPropWithAccess(fmt_, "links", 		config_.linkedLibraries.self);
	appendPropWithAccess(fmt_, "includedirs", 	config_.includeFolders.self);
	appendPropWithAccess(fmt_, "libdirs", 		config_.linkerFolders.self);
}

/////////////////////////////////////////////////
template <typename T>
void appendPropWithAccess(OutputFormatter &fmt_, std::string_view propName, T const& values_)
{
	if (getNumElements(values_) > 0)
	{
		fmt_.write("{} ({{\n", propName);
		{
			IndentScope indent{fmt_};
			appendStringsWithAccess(fmt_, values_);
		}
		fmt_.write("}})\n");
	}
}


/////////////////////////////////////////////////
template <typename T>
void appendStringsWithAccess(OutputFormatter &fmt_, T const& acc_)
{
	for(auto const* acc : getAccesses(acc_))
	{
		if (acc->size() > 0)
		{
			appendStrings(fmt_, *acc);
			fmt_.write("\n");
		}
	}
}

/////////////////////////////////////////////////
void appendStrings(OutputFormatter &fmt_, VecOfStr const& vec_)
{
	for(auto const & str : vec_)
		fmt_.write("\"{}\",\n", str);
}


/////////////////////////////////////////////////
template <typename T>
typename Dict<T>::const_iterator mapString(Dict<T> const& dict_, std::string_view v)
{
	for(auto it = dict_.begin(); it != dict_.end(); it++)
	{
		if (compareIgnoreCase(std::get<0>(*it), v))
			return it;
	}
	return dict_.end();
}



}
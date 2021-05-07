#include PACC_PCH

#include <Pacc/Generators/Premake5.hpp>
#include <Pacc/OutputFormatter.hpp>
#include <Pacc/Filesystem.hpp>
#include <Pacc/Environment.hpp>
#include <Pacc/BuildQueueBuilder.hpp>

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
void appendProject			(OutputFormatter &fmt_, Project const& project_);
template <typename T>
void appendPropWithAccess	(OutputFormatter &fmt_, std::string_view propName, T const& values_);
template <typename T>
void appendStringsWithAccess(OutputFormatter &fmt_, T const& vec_);

template <typename T, typename TMapValueFn = std::nullptr_t>
void mergeFields(std::vector<T>& into_, std::vector<T> const& from_, TMapValueFn mapValueFn_ = nullptr);

template <typename T, typename TMapValueFn = std::nullptr_t>
void mergeAccesses(T &into_, T const & from_, AccessType method_, TMapValueFn mapValueFn_ = nullptr);

/////////////////////////////////////////////////
template <typename T>
typename Dict<T>::const_iterator mapString(Dict<T> const& dict_, std::string_view v);

/////////////////////////////////////////////////
bool compareIgnoreCase(std::string_view l, std::string_view r);

/////////////////////////////////////////////////
void Premake5::generate(Package & pkg_)
{
	// Prepare output buffer
	std::string out;
	out.reserve(4 * 1024 * 1024);

	OutputFormatter fmt{out};

	BuildQueueBuilder cfgQueue;

	cfgQueue.recursiveLoad(pkg_);

	auto const& q = cfgQueue.setup();

	// fmt::print("Configuration steps: {}\n", q.size());
	// size_t stepCounter = 0;
	for(auto & step : q)
	{
		// fmt::print("Step {}: [ ", stepCounter++);
		// size_t depCounter = 0;
		for(auto & dep : step)
		{
			// if (depCounter++ != 0)
			// 	fmt::print(", ");
			// fmt::print("\"{}\"", dep.project->name);

			auto& pkgDep = dep.dep->package();

			auto resolvePath = [&](auto const& pathElem) {
					fs::path path = fs::u8path(pathElem);
					if (path.is_relative())
						return fsx::fwd(pkgDep.package->root.parent_path() / path).string();
					else 
						return pathElem;
				};
			for (auto const & depProjName : pkgDep.projects)
			{
				Project const* remoteProj = pkgDep.package->findProject(depProjName);

				mergeAccesses(dep.project->defines, 			remoteProj->defines, 			dep.dep->accessType);
				mergeAccesses(dep.project->includeFolders, 		remoteProj->includeFolders, 	dep.dep->accessType, resolvePath);
				mergeAccesses(dep.project->linkerFolders, 		remoteProj->linkerFolders, 		dep.dep->accessType, resolvePath);

				// Add dependency output folder:
				{
					auto& target = targetByAccessType(dep.project->linkerFolders.computed, dep.dep->accessType);
					target.push_back(fsx::fwd(pkgDep.package->predictOutputFolder(*remoteProj)).string());
				}
				
				mergeAccesses(dep.project->linkedLibraries, 	remoteProj->linkedLibraries, 	dep.dep->accessType);

				// Add dependency file to linker:
				{
					auto& target = targetByAccessType(dep.project->linkedLibraries.computed, dep.dep->accessType);
					target.push_back(remoteProj->name);
				}
			}

		}
		// fmt::print(" ]\n", stepCounter++);
	}


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
			appendProject(fmt_, project);
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
void appendProject(OutputFormatter &fmt_, Project const& project_)
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

		// TODO: Refactor this code

		// Computed:
		appendPropWithAccess(fmt_, "defines", 		project_.defines.computed);
		appendPropWithAccess(fmt_, "links", 		project_.linkedLibraries.computed);
		appendPropWithAccess(fmt_, "includedirs", 	project_.includeFolders.computed);
		appendPropWithAccess(fmt_, "libdirs", 		project_.linkerFolders.computed);

		
		appendPropWithAccess(fmt_, "files", 		project_.files);
		appendPropWithAccess(fmt_, "defines", 		project_.defines.self);
		appendPropWithAccess(fmt_, "links", 		project_.linkedLibraries.self);
		appendPropWithAccess(fmt_, "includedirs", 	project_.includeFolders.self);
		appendPropWithAccess(fmt_, "libdirs", 		project_.linkerFolders.self);

	}
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


/////////////////////////////////////////////////
template <typename T, typename TMapValueFn>
void mergeFields(std::vector<T>& into_, std::vector<T> const& from_, TMapValueFn mapValueFn_)
{
	// Do not map values
	if constexpr (std::is_same_v<TMapValueFn, std::nullptr_t>)
	{
		into_.insert(
				into_.end(),
				from_.begin(),
				from_.end()
			);
	}
	else
	{
		into_.reserve(from_.size());
		for(auto const & elem : from_)
		{
			into_.push_back( mapValueFn_(elem));
		}
	}
}

/////////////////////////////////////////////////
template <typename T, typename TMapValueFn>
void mergeAccesses(T &into_, T const & from_, AccessType method_, TMapValueFn mapValueFn_)
{
	
	auto& target = targetByAccessType(into_.computed, method_); // by default

	auto forBoth =
		[](auto & selfAndComputed, auto const& whatToDo)
		{
			whatToDo(selfAndComputed.computed);
			whatToDo(selfAndComputed.self);
		};

	// Private is private
	// Merge only interface and public:
	auto mergeFieldsTarget =
		[&](auto &selfOrComputed)
		{
			mergeFields(target, selfOrComputed.interface_, mapValueFn_);
			mergeFields(target, selfOrComputed.public_, mapValueFn_);
		};

	forBoth(from_, mergeFieldsTarget);
}

/////////////////////////////////////////////////
bool compareIgnoreCase(std::string_view l, std::string_view r)
{
	if (l.length() != r.length()) return false;

	for(std::size_t i = 0; i < l.size(); i++)
	{
		if ( std::tolower(int(l[i])) != std::tolower(int(r[i])) )
			return false;
	}

	return true;
}


}
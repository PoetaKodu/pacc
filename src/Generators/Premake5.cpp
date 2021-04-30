#include <CppPkg/Generators/Premake5.hpp>
#include <fmt/core.h>
#include <fstream>
#include <array>

using namespace fmt;



namespace gen
{

struct Formatter
{
	std::string& output;	
	int indent = 0;

	void writeIndent()
	{
		for(int i = 0; i < indent; ++i)
			output += '\t';
	}

	template <bool Indent = true, typename FirstArg, typename... Args>
	void write(FirstArg&& firstArg_, Args&&... args_)
	{
		if constexpr (Indent) {
			this->writeIndent();
		}
		output += format( std::forward<FirstArg>(firstArg_), std::forward<Args>(args_)... );
	}
};

struct IndentScope
{
	Formatter& fmt;
	IndentScope(Formatter& fmt_)
		: fmt(fmt_)
	{
		++fmt.indent;
	}
	~IndentScope() {
		--fmt.indent;
	}
};

/////////////////////////////////////////////////
auto getNumElements(VecOfStr const& v)
{
	return v.size();
}

/////////////////////////////////////////////////
auto getNumElements(VecOfStrAcc const& v)
{
	return v.public_.size() + v.private_.size() + v.interface_.size();
}


/////////////////////////////////////////////////
auto getAccesses(VecOfStr const& v)
{
	return std::vector{ &v };
}

/////////////////////////////////////////////////
auto getAccesses(VecOfStrAcc const& v)
{
	return std::vector{ &v.private_, &v.public_, &v.interface_ };
}


/////////////////////////////////////////////////
template <typename T>
concept Accessible = requires(T t) {
	{ getAccesses(t) };
	{ getNumElements(t) };
};

void appendWorkspace		(Formatter &fmt_, Package const& pkg_);
void appendProject			(Formatter &fmt_, Project const& project_);
void appendPropWithAccess	(Formatter &fmt_, std::string_view propName, Accessible auto const& values_);
void appendStringsWithAccess(Formatter &fmt_, Accessible auto const& vec_);


/////////////////////////////////////////////////
void Premake5::generate(Package const& pkg_)
{
	// Prepare output buffer
	std::string out;
	out.reserve(4 * 1024 * 1024);

	Formatter fmt{out};

	appendWorkspace(fmt, pkg_);

	// Store the output in the premake file
	std::ofstream("premake5.lua") << out;
}

/////////////////////////////////////////////////
void appendWorkspace(Formatter &fmt_, Package const& pkg_)
{
	fmt_.write("workspace(\"{}\")\n", pkg_.name);

	{
		IndentScope indent{fmt_};

		// TODO: add support for user configurations
		{
			fmt_.write("configurations(\"debug\")\n");

			IndentScope indent{fmt_};
			fmt_.write("symbols(\"On\")\n");
		}

		// TODO: add support for user configurations
		{
			fmt_.write("configurations(\"release\")\n");

			IndentScope indent{fmt_};
			fmt_.write("symbols(\"Off\")\n\n");
		}
		
		for(auto const& project : pkg_.projects)
		{
			appendProject(fmt_, project);
		}
	}
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


template <typename T = std::string_view>
using DictElem 	= std::pair<std::string_view, T>;
template <typename T = std::string_view>
using Dict 		= std::vector< DictElem<T> >;


/////////////////////////////////////////////////
template <typename T>
auto mapString(Dict<T> const& dict_, std::string_view v)
{
	for(auto it = dict_.begin(); it != dict_.end(); it++)
	{
		if (compareIgnoreCase(std::get<0>(*it), v))
			return it;
	}
	return dict_.end();
}

/////////////////////////////////////////////////
std::string_view mapToPremake5Kind(std::string_view projectType_)
{
	static const Dict<> PremakeKind = {
		{ "app", 		"ConsoleApp" },
		{ "static lib", "StaticLib" },
		{ "shared lib", "SharedLib" }
	};

	auto it = mapString(PremakeKind, projectType_);
	if (it != PremakeKind.end())
		return it->second;
	
	return "";
}

/////////////////////////////////////////////////
void appendPremake5Lang(Formatter& fmt_, std::string_view lang_)
{
	using LangAndDialect = std::pair<std::string_view, std::string_view>;
	static const Dict<LangAndDialect> PremakeLangAndDialect = {
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

	auto it = mapString(PremakeLangAndDialect, lang_);
	if (it != PremakeLangAndDialect.end())
	{
		auto const& premakeVal = it->second;
		
		fmt_.write("language (\"{}\")\n", premakeVal.first);
		if (!premakeVal.second.empty())
			fmt_.write("cppdialect (\"{}\")\n", premakeVal.second);
	}
}



/////////////////////////////////////////////////
void appendProject(Formatter &fmt_, Project const& project_)
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
			fmt_.write("language(\"C++\")\n", project_.language);
			fmt_.write("cppdialect(\"C++17\")\n", project_.cppStandard);
		}

		appendPropWithAccess(fmt_, "files", 		project_.files);
		appendPropWithAccess(fmt_, "links", 		project_.linkedLibraries);
		appendPropWithAccess(fmt_, "includedirs", 	project_.includeFolders);
		appendPropWithAccess(fmt_, "libdirs", 		project_.linkerFolders);
	}
}



/////////////////////////////////////////////////
void appendPropWithAccess(Formatter &fmt_, std::string_view propName, Accessible auto const& values_)
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
void appendStringsWithAccess(Formatter &fmt_, Accessible auto const& acc_)
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
void appendStrings(Formatter &fmt_, VecOfStr const& vec_)
{
	for(auto const & str : vec_)
		fmt_.write("\"{}\",\n", str);
}


}
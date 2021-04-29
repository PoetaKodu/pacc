#include <CppPkg/Generators/Premake5.hpp>
#include <fmt/core.h>
#include <fstream>

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

	for(auto const& project : pkg_.projects)
	{
		IndentScope indent{fmt_};
		appendProject(fmt_, project);
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

		appendPropWithAccess(fmt_, "files", 		project_.files);
		appendPropWithAccess(fmt_, "includedirs", 	project_.includeFolders);
		appendPropWithAccess(fmt_, "links", 		project_.linkedLibraries);
		appendPropWithAccess(fmt_, "linkdirs", 		project_.linkerFolders);
	}
}



/////////////////////////////////////////////////
void appendPropWithAccess(Formatter &fmt_, std::string_view propName, Accessible auto const& values_)
{
	fmt_.write("{} ({{\n", propName);
	{
		IndentScope indent{fmt_};
		appendStringsWithAccess(fmt_, values_);
	}
	fmt_.write("}})\n");
}



/////////////////////////////////////////////////
void appendStringsWithAccess(Formatter &fmt_, Accessible auto const& acc_)
{
	for(auto const* acc : getAccesses(acc_))
	{
		appendStrings(fmt_, *acc);

		fmt_.write("\n");
	}
}

/////////////////////////////////////////////////
void appendStrings(Formatter &fmt_, VecOfStr const& vec_)
{
	for(auto const & str : vec_)
		fmt_.write("\"{}\",\n", str);
}


}
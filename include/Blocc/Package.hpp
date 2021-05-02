#pragma once

#include BLOCC_PCH

constexpr std::string_view PackageJSON 	= "cpackage.json";
constexpr std::string_view PackageLUA 	= "cpackage.lua";

using VecOfStr 		= std::vector< std::string >;
using PackagePtr 	= std::shared_ptr<struct Package>;
struct VecOfStrAcc
{
	VecOfStr public_;
	VecOfStr private_;
	VecOfStr interface_;
};

struct Dependency
{
	std::string packageName;
	std::string projectName;

	// Resolved (or not) package pointer.
	PackagePtr 	package;
	// std::string version; // TODO add version support
	// TODO: add config support

	static Dependency from(std::string_view depPattern);
};

struct TargetBase
{
	std::string 	name;

	VecOfStr 				files;
	std::vector<Dependency> dependencies;
	VecOfStrAcc 			defines;
	VecOfStrAcc 			includeFolders;
	VecOfStrAcc 			linkerFolders;
	VecOfStrAcc 			linkedLibraries;
};

struct Project : TargetBase
{
	std::string		type;
	std::string		language;
	std::string		cppStandard;
	std::string		cStandard;
};

struct Package : TargetBase
{
	fs::path 		root;
	std::vector<Project> projects;

	static Package load(fs::path dir_ = "");

private:
	static Package loadFromJSON(std::string const& packageContent_);
};





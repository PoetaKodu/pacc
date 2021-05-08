#pragma once

#include PACC_PCH

#include <PACC/Helpers/HelperTypes.hpp>
#include <Pacc/PackageSystem/Dependency.hpp>

constexpr std::string_view PackageJSON 	= "cpackage.json";
constexpr std::string_view PackageLUA 	= "cpackage.lua";

using PackagePtr 	= std::shared_ptr<struct Package>;

/////////////////////////////////////////////////
std::size_t getNumElements(VecOfStr const& v);

/////////////////////////////////////////////////
std::size_t getNumElements(VecOfStrAcc const& v);


struct TargetBase
{
	std::string 	name;

	template <typename T>
	struct SelfAndComputed {
		T self;
		T computed;
	};
	template <typename T>
	using SaC = SelfAndComputed<T>;

	VecOfStr					 	files;
	SaC<AccessSplitVec<Dependency>> dependencies;
	SaC<VecOfStrAcc>			 	defines;
	SaC<VecOfStrAcc>			 	includeFolders;
	SaC<VecOfStrAcc>			 	linkerFolders;
	SaC<VecOfStrAcc>			 	linkedLibraries;
};

struct Project : TargetBase
{
	std::string		type;
	std::string		language;
};

struct Package : TargetBase
{
	fs::path 				root;
	std::vector<Project> 	projects;

	static Package load(fs::path dir_ = "");
	static Package loadByName(std::string_view name_);

	Project const* findProject(std::string_view name_) const;

	fs::path predictOutputFolder(Project const& project_) const;
	fs::path predictRealOutputFolder(Project const& project_) const;
private:
	static Package loadFromJSON(std::string const& packageContent_);
};



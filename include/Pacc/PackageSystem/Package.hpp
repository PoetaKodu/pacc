#pragma once

#include PACC_PCH

#include <PACC/Helpers/HelperTypes.hpp>
#include <Pacc/PackageSystem/Dependency.hpp>
#include <Pacc/Toolchains/Toolchain.hpp>

struct Package;
struct Project;

constexpr std::string_view PackageJSON 	= "cpackage.json";
constexpr std::string_view PackageLUA 	= "cpackage.lua";

using PackagePtr 	= std::shared_ptr<Package>;

/////////////////////////////////////////////////
std::size_t getNumElements(VecOfStr const& v);

/////////////////////////////////////////////////
std::size_t getNumElements(VecOfStrAcc const& v);

struct Configuration
{
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
struct TargetBase : Configuration
{
	std::string 	name;

	Map<std::string, Configuration> premakeFilters;

	void inheritConfigurationFrom(Package const& fromPkg_, Project const& fromProject_, AccessType mode_);
};

struct PrecompiledHeader
{
	std::string 		header;
	std::string 		source;
	std::string 		definition;
};

struct Project : TargetBase
{
	std::string type;
	std::string language;
	std::optional<PrecompiledHeader> pch;
};

struct Package : TargetBase
{
	fs::path 				root;
	std::vector<Project> 	projects;

	static Package load(fs::path dir_ = "");
	static Package loadByName(std::string_view name_);

	Project const* findProject(std::string_view name_) const;

	Project const& requireProject(std::string_view name_) const;

	fs::path predictOutputFolder(Project const& project_) const;
	fs::path predictRealOutputFolder(Project const& project_, BuildSettings settings_ = {}) const;

	/////////////////////////////////////////////////
	fs::path resolvePath( fs::path const& path_) const;

private:
	static Package loadFromJSON(std::string const& packageContent_);
};


template <typename T, typename TMapValueFn = ReturnIdentity>
void mergeFields(std::vector<T>& into_, std::vector<T> const& from_, TMapValueFn&& mapValueFn_ = TMapValueFn())
{
	into_.reserve(from_.size());
	for(auto const & elem : from_)
	{
		into_.push_back( mapValueFn_(elem));
	}	
}

template <typename T, typename TMapValueFn = ReturnIdentity>
void mergeAccesses(T &into_, T const & from_, AccessType method_, TMapValueFn&& mapValueFn_ = TMapValueFn())
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

/// <summary>Merges configuration into target configuration (computed field).</summary>
/// <param name="into_">Target configuration</param>
/// <param name="fromPkg_">Source package</param>
/// <param name="fromProject_">Source project</param>
/// <param name="from_">Source configuration</param>
/// <param name="mode_">Merge mode</param>
void computeConfiguration(Configuration& into_, Package const& fromPkg_, Project const& fromProject_, Configuration const& from_, AccessType mode_);

/// <summary>Merges project configuration into target configuration (computed field).</summary>
/// <param name="into_">Target configuration</param>
/// <param name="fromPkg_">Source package</param>
/// <param name="fromProject_">Source project</param>
/// <param name="mode_">Merge mode</param>
/// <remarks>A shorthand for calling the same function, where source configuration is equal to the project's default</remarks>
inline void computeConfiguration(
		Configuration& 	into_,
		Package const& 	fromPkg_,
		Project const& 	fromProject_,
		AccessType 		mode_
	)
{
	computeConfiguration(into_, fromPkg_, fromProject_, fromProject_, mode_);
}
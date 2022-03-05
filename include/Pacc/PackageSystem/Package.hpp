#pragma once

#include PACC_PCH

#include <Pacc/Helpers/HelperTypes.hpp>
#include <Pacc/PackageSystem/Dependency.hpp>
#include <Pacc/Toolchains/Toolchain.hpp>

struct Package;
struct Project;

constexpr std::string_view PackageJSON		= "cpackage.json";
constexpr std::string_view PackageLUA		= "cpackage.lua";
constexpr std::string_view PackageLUAScript	= "cpackage.script.lua";

using PackagePtr 	= std::shared_ptr<Package>;

/////////////////////////////////////////////////
std::size_t getNumElements(VecOfStr const& v);

/////////////////////////////////////////////////
std::size_t getNumElements(VecOfStrAcc const& v);


/// <summary>GNU visibility mode</summary>
/// <remarks>Check https://gcc.gnu.org/wiki/Visibility</remarks>
struct GNUSymbolVisibility
{
	enum Mode
	{
		Default,
		Hidden,
		Inline,
		Unknown
	} value = Mode::Default;

	operator Mode() const {
		return value;
	}

	static GNUSymbolVisibility fromString(std::string_view str_)
	{
		if (compareIgnoreCase(str_, "Default"))
			return { Mode::Default };
		else if (compareIgnoreCase(str_, "Hidden"))
			return { Mode::Hidden };
		else if (compareIgnoreCase(str_, "Inline"))
			return { Mode::Inline };

		return { Mode::Unknown };
	}

	std::string_view toString() const
	{
		if (value == Mode::Default)
			return "Default";
		else if (value == Mode::Hidden)
			return "Hidden";

		return "Inline";
	}
};

struct Command
{
	Command(std::string content_, bool req_ = true)
		:
		content(std::move(content_)),
		required(req_)
	{

	}

	std::string content;
	bool required = true;
};


struct Configuration
{
	template <typename T>
	struct SelfAndComputed {
		T self;
		T computed;
	};
	template <typename T>
	using SaC = SelfAndComputed<T>;


	GNUSymbolVisibility 			symbolVisibility;

	std::string 					moduleDefinitionFile;
	VecOfStr					 	files;
	SaC<AccessSplitVec<Dependency>> dependencies;
	SaC<VecOfStrAcc>			 	defines;
	SaC<VecOfStrAcc>			 	includeFolders;
	SaC<VecOfStrAcc>			 	linkerFolders;
	SaC<VecOfStrAcc>			 	linkedLibraries;
	SaC<VecOfStrAcc>			 	compilerOptions;
	SaC<VecOfStrAcc>			 	linkerOptions;
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

enum class ProjectType
{
	App,
	StaticLib,
	SharedLib,
	Interface,
	Unknown
};

std::string toString(ProjectType type_);
ProjectType parseProjectType(std::string_view type_);

struct Project : TargetBase
{
	using Type = ProjectType;

	std::string language;
	std::optional<PrecompiledHeader> pch;

	Type type;

	bool isLibrary() const {
		return type == Type::StaticLib || type == Type::SharedLib;
	}
};
struct PackagePreloadInfo
{
	fs::path root;
	fs::path scriptFile;

	bool usesJsonConfig() const {
		return root.filename() == PackageJSON;
	}

	bool usesLuaConfig() const {
		return root.filename() == PackageLUA;
	}

	bool usesScriptFile() const {
		return !scriptFile.empty();
	}
};

struct Package
	:
	TargetBase,
	PackagePreloadInfo
{
	std::vector<Project> 	projects;
	Version					version;
	std::string 			startupProject;

	static PackagePreloadInfo preload(fs::path dir_ = "");

	static UPtr<Package> load(fs::path dir_ = "")
	{
		return load( preload(dir_) );
	}

	static UPtr<Package> load(PackagePreloadInfo info_);

	static UPtr<Package> loadByName(std::string_view name_, VersionRequirement verReq_ = {}, UPtr<Package>* invalidVersion_ = nullptr);

	Project const* findProject(std::string_view name_) const;

	Project const& requireProject(std::string_view name_) const;

	fs::path predictOutputFolder(Project const& project_) const;
	fs::path predictRealOutputFolder(Project const& project_, BuildSettings settings_ = {}) const;

	/////////////////////////////////////////////////
	fs::path resolvePath( fs::path const& path_) const;



private:
	static bool loadFromJSON(Package& package_, std::string const& packageContent_);
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

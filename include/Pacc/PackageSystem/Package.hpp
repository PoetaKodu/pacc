#pragma once

#include <Pacc/PaccPCH.hpp>

#include <Pacc/Helpers/HelperTypes.hpp>
#include <Pacc/PackageSystem/Dependency.hpp>
#include <Pacc/Toolchains/Toolchain.hpp>

struct Package;
struct Project;

constexpr StringView PackageJSON[2]		= { "pacc.json", "cpackage.json" };
constexpr StringView PackageLUA[2]		= { "pacc.lua", "cpackage.lua" };
constexpr StringView PackageLUAScript[2]	= { "pacc.script.lua", "cpackage.script.lua" };

using PackagePtr 	= std::shared_ptr<Package>;

auto getNumElements(Vec<String> const& v) -> std::size_t;
auto getNumElements(VecOfStrAcc const& v) -> std::size_t;


auto findPackageFile(fs::path const& directory_, Opt<StringView> extension_ = std::nullopt) -> fs::path;
auto findPackageScriptFile(fs::path const& directory_) -> fs::path;

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

	static auto fromString(StringView str_) -> GNUSymbolVisibility
	{
		if (compareIgnoreCase(str_, "Default"))
			return { Mode::Default };
		else if (compareIgnoreCase(str_, "Hidden"))
			return { Mode::Hidden };
		else if (compareIgnoreCase(str_, "Inline"))
			return { Mode::Inline };

		return { Mode::Unknown };
	}

	auto toString() const -> StringView
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
	Command(String content_, bool req_ = true)
		:
		content(std::move(content_)),
		required(req_)
	{

	}

	String content;
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

	String 					moduleDefinitionFile;
	Vec<String>					 	files;
	SaC<AccessSplitVec<Dependency>> dependencies;
	SaC<VecOfStrAcc>			 	defines;
	SaC<VecOfStrAcc>			 	includeFolders;
	SaC<VecOfStrAcc>			 	linkerFolders;
	SaC<VecOfStrAcc>			 	linkedLibraries;
	SaC<VecOfStrAcc>			 	compilerOptions;
	SaC<VecOfStrAcc>			 	linkerOptions;
};

struct ScriptableAction
{
	String moduleName; // either a file or a plugin
	String functionName;
};

using OptScriptableAction	= Opt<ScriptableAction>;
using ScriptableActionsMap	= std::unordered_map<String, OptScriptableAction>;

struct ScriptableTarget {
	ScriptableActionsMap scripts;
};

enum class Artifact {
	Executable,				// typically "*.exe", "*.out" or without extension
	Library,				// typically "*.dll", "*.so" or "*.a"
	LibraryInterface,		// typically "*.lib"
	DebugSymbols,			// typically "*.pdb"

	Unknown,
	MAX
};
inline constexpr auto ArtifactTypesCount = static_cast<int>(Artifact::MAX);

Artifact detectArtifactTypeFromPath(StringView path_);

struct ArtifactProducer {
	using ArtifactsType = Array<Vec<fs::path>, ArtifactTypesCount>;

	ArtifactsType artifacts;
};

struct TargetBase
	:
	Configuration,
	ArtifactProducer
{
	String 	name;

	Map<String, Configuration>	premakeFilters;

	void inheritConfigurationFrom(Package const& fromPkg_, Project const& fromProject_, AccessType mode_);

	fs::path outputArtifact() const;
};

struct PrecompiledHeader
{
	String 		header;
	String 		source;
	String 		definition;
};

enum class ProjectType
{
	App,
	StaticLib,
	SharedLib,
	Interface,
	HandledByPlugin,
	Unknown
};

auto toString(ProjectType type_, StringView pluginName_ = "") -> String;
auto parseProjectType(StringView type_) -> ProjectType;

struct Project
	: TargetBase, ScriptableTarget
{
	using Type = ProjectType;
	using enum ProjectType;

	String language;
	Opt<PrecompiledHeader> pch;

	Type type;

	auto getPrimaryArtifactOfType(Artifact artType_) const -> fs::path;
	auto getLinkTargetArtifact() const -> fs::path;
	auto getPrimaryArtifact() const -> fs::path;

	auto isLibrary() const -> bool;
};

struct PackagePreloadInfo
{
	fs::path root;
	fs::path scriptFile;

	bool usesJsonConfig() const {
		return rg::find(PackageJSON, root.filename()) != std::end(PackageJSON);
	}

	bool usesLuaConfig() const {
		return rg::find(PackageJSON, root.filename()) != std::end(PackageLUA);
	}

	bool usesScriptFile() const {
		return !scriptFile.empty();
	}
};

struct Package
	:
	TargetBase,
	ScriptableTarget,
	PackagePreloadInfo
{
	Vec<Project> 	projects;
	Version			version;
	bool			isCMake = false;
	String 			startupProject;

	static PackagePreloadInfo preload(fs::path dir_ = "");

	void loadPackageSpecificInfo(json const& json_);
	void loadWorkspaceInfo(json const& json_);

	static UPtr<Package> load(fs::path dir_ = "")
	{
		return load( preload(dir_) );
	}

	static UPtr<Package> load(PackagePreloadInfo info_);

	auto findProject(StringView name_) const -> Project const*;
	auto requireProject(StringView name_) const -> Project const&;

	auto predictOutputFolder(Project const& project_) const -> fs::path;
	auto predictRealOutputFolder(Project const& project_, BuildSettings settings_ = {}) const -> fs::path;

	auto getAbsoluteArtifactFilePath(Project const& project_) const -> fs::path;

	auto resolvePath( fs::path const& path_) const -> fs::path;

	fs::path outputRoot;

	IPackageBuilder* builder = nullptr; // nullptr - use default builder

	auto rootFolder() const {
		return this->root.has_extension() ? this->root.parent_path() : this->root;
	}

private:
	static bool loadFromJSON(Package& package_, String const& packageContent_);
};


template <typename T, typename TMapValueFn = ReturnIdentity>
void mergeFields(Vec<T>& into_, Vec<T> const& from_, TMapValueFn&& mapValueFn_ = TMapValueFn())
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

#pragma once

#include PACC_PCH

#include <Pacc/Main.hpp>
#include <Pacc/App/PaccConfig.hpp>
#include <Pacc/PackageSystem/Package.hpp>
#include <Pacc/PackageSystem/IPackageLoader.hpp>
#include <Pacc/Generation/Premake5.hpp>
#include <Pacc/Toolchains/Toolchain.hpp>
#include <Pacc/Generation/BuildQueueBuilder.hpp>

class PaccApp
{
public:
	constexpr static std::string_view PaccVersion = "0.3.2-prealpha";

	// Actions:
	// help
	void 			displayHelp(bool full_);
	// link
	void 			linkPackage();
	// unlink
	void 			unlinkPackage();
	// generate
	void 			generate();
	// toolchains
	void 			toolchains();
	// build
	void 			buildPackage();
	// run
	void 			run();
	// init
	void 			initPackage();
	// logs
	void 			logs();
	// install
	void 			install();
	// uninstall
	void 			uninstall();
	// list-versions
	void 			listVersions();
	// graph
	void			visualizeGraph();
	// query
	void			query();



	///////////////////////
	// Other functions:
	///////////////////////
	UPtr<Package>	loadPackage(fs::path const& path_);
	UPtr<Package>	loadPackage(fs::path const& path_, std::string const& loaderName_);
	UPtr<Package>	loadPackageByName(std::string const& name_, VersionRequirement verReq_ = {}, UPtr<Package>* invalidVersion_ = nullptr, std::string const& loaderName_ = "auto");

	IPackageLoader& detectPreferredPackageLoaderFor(fs::path const& path_) const;

	void 			loadPaccConfig();

	PaccApp();

	ProgramArgs		args;
	PaccConfig 		cfg;

	sol::state		lua;

	UMap<std::string, UPtr<IPackageLoader> > packageLoaders;
	using AutodetectPackageLoaderQueue = std::priority_queue<
			IPackageLoader*,
			std::vector<IPackageLoader*>,
			decltype(PackageLoaderAutodetectPriorityComp)
		>;

	IPackageLoader* defaultPackageLoader;
	AutodetectPackageLoaderQueue autodetectPackageLoaders;

	IPackageLoader* registerPackageLoader(std::string const& name_, UPtr<IPackageLoader> loader_);

	// TODO: use date instead of amount
	void 			cleanupLogs(size_t maxLogs_) const;

	auto execLuaEvent(Package& pkg, std::string const& funcName_) -> sol::protected_function_result;

private:
	auto setupPackageLoaders() -> void;
	auto setupLua() -> void;
	auto createPremake5Generator() -> gen::Premake5;
	auto determineBuildSettingsFromArgs() const -> BuildSettings;
	auto buildSpecifiedPackage(Package& pkg_, Toolchain& toolchain_, BuildSettings const& settings_, bool isDependency_ = false) -> void;
	auto installPackageDependencies(Package& pkg_, bool isRoot) -> size_t;

	/// <summary>
	/// 	Determines whether program arguments contain
	/// 	specified <paramref name="switch_"/>
	/// </summary>
	/// <param name="switch_">Tested switch, for example "--test"</param>
	/// <returns><c>true</c> if found otherwise <c>false</c></returns>
	auto containsSwitch(std::string_view switch_) const -> bool;

	auto argValue(std::string_view name_) const -> std::string;

	auto downloadPackage(fs::path const &target_, DownloadLocation const& loc_) -> void;

	auto ensureProjectsAreBuilt(Package& pkg_, std::vector<std::string> const& projectNames_, BuildSettings const& settings_) -> void;
	auto ensureDependenciesBuilt(Package& pkg_, BuildQueueBuilder const &depQueue_, BuildSettings const& settings_) -> void;

	auto collectMissingDependencies(Package const & pkg_) -> std::vector<PackageDependency>;
};

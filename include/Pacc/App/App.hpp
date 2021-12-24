#pragma once

#include PACC_PCH

#include <Pacc/Main.hpp>
#include <Pacc/App/PaccConfig.hpp>
#include <Pacc/PackageSystem/Package.hpp>
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

	///////////////////////
	// Other functions:
	///////////////////////

	void 			loadPaccConfig();

	PaccApp();

	ProgramArgs		args;
	PaccConfig 		cfg;

	sol::state		lua;

	// TODO: use date instead of amount
	void 			cleanupLogs(size_t maxLogs_) const;

private:
	void			setupLua();

	gen::Premake5 	createPremake5Generator();

	BuildSettings 	determineBuildSettingsFromArgs() const;

	void 			buildSpecifiedPackage(Package const & pkg_, Toolchain& toolchain_, BuildSettings const& settings_, bool isDependency_ = false);

	size_t 			installPackageDependencies(Package& pkg_, bool isRoot);

	/// <summary>
	/// 	Determines whether program arguments contain
	/// 	specified <paramref name="switch_"/>
	/// </summary>
	/// <param name="switch_">Tested switch, for example "--test"</param>
	/// <returns><c>true</c> if found otherwise <c>false</c></returns>
	bool containsSwitch(std::string_view switch_) const;

	void downloadPackage(fs::path const &target_, DownloadLocation const& loc_);

	void ensureProjectsAreBuilt(Package const& pkg_, std::vector<std::string> const& projectNames_, BuildSettings const& settings_);
	void ensureDependenciesBuilt(Package const& pkg_, BuildQueueBuilder const &depQueue_, BuildSettings const& settings_);

	std::vector<PackageDependency> collectMissingDependencies(Package const & pkg_);
};

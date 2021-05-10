#pragma once

#include PACC_PCH

#include <Pacc/Main.hpp>
#include <Pacc/App/PaccConfig.hpp>
#include <Pacc/PackageSystem/Package.hpp>
#include <Pacc/Toolchains/Toolchain.hpp>

class PaccApp
{
public:
	// Actions:
	// help
	void 			displayHelp(bool full_);
	// link
	void 			linkPackage();
	// unlink
	void 			unlinkPackage();
	// generate
	Package 		generate();
	// toolchains
	void 			toolchains();
	// build
	void 			buildPackage();
	// run
	void 			runPackageStartupProject();
	// init
	void 			initPackage();
	// logs
	void 			logs();

	///////////////////////
	// Other functions:
	///////////////////////

	void 			loadPaccConfig();

	ProgramArgs		args;
	PaccConfig 		cfg;

	// TODO: use date instead of amount
	void 			cleanupLogs(size_t maxLogs_) const;

private:
	BuildSettings 	determineBuildSettingsFromArgs() const;

	/// <summary>
	/// 	Determines whether program arguments contain
	/// 	specified <paramref name="switch_"/>
	/// </summary>
	/// <param name="switch_">Tested switch, for example "--test"</param>
	/// <returns><c>true</c> if found otherwise <c>false</c></returns>
	bool containsSwitch(std::string_view switch_) const;
};
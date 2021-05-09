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

	ProgramArgs		args;
	PaccConfig 		cfg;

private:
	BuildSettings 	determineBuildSettingsFromArgs() const;
};
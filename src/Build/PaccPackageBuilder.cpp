#include "include/Pacc/PaccPCH.hpp"

#include <Pacc/Build/PaccPackageBuilder.hpp>

#include <Pacc/Helpers/Exceptions.hpp>
#include <Pacc/Toolchains/Toolchain.hpp>
#include <Pacc/Generation/Premake5.hpp>
#include <Pacc/App/App.hpp>

///////////////////////////////////////////
auto PaccPackageBuilder::run(
		Package const&			package,
		Toolchain&				toolchain,
		BuildSettings const&	settings,
		int						verbosityLevel
	) -> BuildProcessResult
{
	// Generate premake5 files
	app->createPremake5Generator().generate(package);

	// Run premake:
	app->runPremakeGeneration(toolchain.premakeToolchainType());

	// TODO: build should be implemented here, instead of in the toolchain
	return toolchain.run(package, settings, verbosityLevel);
}

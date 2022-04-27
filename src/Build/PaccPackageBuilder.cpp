#include PACC_PCH

#include <Pacc/Build/PaccPackageBuilder.hpp>

#include <Pacc/Helpers/Exceptions.hpp>
#include <Pacc/Toolchains/Toolchain.hpp>
#include <Pacc/Generation/Premake5.hpp>
#include <Pacc/App/App.hpp>

///////////////////////////////////////////
BuildProcessResult PaccPackageBuilder::run(Package const& pkg_, Toolchain& tc_, BuildSettings const& settings_, int verbosityLevel_)
{
	// Generate premake5 files
	app->createPremake5Generator().generate(pkg_);

	// Run premake:
	gen::runPremakeGeneration(tc_.premakeToolchainType());

	// TODO: build should be implemented here, instead of in the toolchain
	return tc_.run(pkg_, settings_, verbosityLevel_);
}

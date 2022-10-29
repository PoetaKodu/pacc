#pragma once

#include PACC_PCH

#include <Pacc/PackageSystem/Package.hpp>
#include <Pacc/Helpers/HelperTypes.hpp>

namespace gen
{

class Premake5
{
	static bool exportCompileCommands();

public:
	bool compileCommands = false; // Should export compile commands?

	void generate(Package const & package_);
};

fs::path getPremake5Path();

void runPremakeGeneration(StringView toolchainName_);

}


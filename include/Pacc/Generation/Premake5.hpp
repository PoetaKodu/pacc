#pragma once

#include PACC_PCH

#include <Pacc/PackageSystem/Package.hpp>

namespace gen
{

class Premake5
{
	static bool exportCompileCommands();

public:
	bool compileCommands = false; // Should export compile commands?

	void generate(Package const & package_);
};

void runPremakeGeneration(fs::path appRoot_, std::string_view toolchainName_);

}


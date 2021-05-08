#pragma once

#include PACC_PCH

#include <Pacc/PackageSystem/Package.hpp>

namespace gen
{

class Premake5
{
public:
	void generate(Package & package_);
};

void runPremakeGeneration(std::string_view toolchainName_);

}


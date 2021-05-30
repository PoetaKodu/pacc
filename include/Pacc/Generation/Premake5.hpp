#pragma once

#include PACC_PCH

#include <Pacc/PackageSystem/Package.hpp>
#include <Pacc/Generation/BuildQueueBuilder.hpp>

namespace gen
{

class Premake5
{
public:
	void generate(Package & package_, BuildQueueBuilder & depQueue_);
};

void runPremakeGeneration(std::string_view toolchainName_);

}


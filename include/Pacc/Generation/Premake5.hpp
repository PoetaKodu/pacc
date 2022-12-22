#pragma once

#include <Pacc/PaccPCH.hpp>

#include <Pacc/PackageSystem/Package.hpp>
#include <Pacc/Helpers/HelperTypes.hpp>

namespace gen
{

struct Premake5Handler
{
	virtual auto getPremake5Path() -> Path = 0;
};

class Premake5
{
	static bool exportCompileCommands();

public:
	bool compileCommands = false; // Should export compile commands?

	void generate(Package const & package_);
};

}


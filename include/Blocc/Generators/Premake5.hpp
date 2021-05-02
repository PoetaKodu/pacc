#pragma once

#include BLOCC_PCH

#include <Blocc/Package.hpp>

namespace gen
{

class Premake5
{
	std::vector<PackagePtr> dependencies;

	void loadDependencies(Package pkg_);
public:
	void generate(Package const& package_);
};

}


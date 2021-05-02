#pragma once

#include <Blocc/Package.hpp>

namespace gen
{

class Premake5
{
	void loadDependencies(Package const& pkg_);
public:
	void generate(Package const& package_);
};

}


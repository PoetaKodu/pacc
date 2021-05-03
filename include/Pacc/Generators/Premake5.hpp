#pragma once

#include PACC_PCH

#include <Pacc/Package.hpp>

namespace gen
{

class Premake5
{
	std::vector<PackagePtr> loadedPackages;
	std::vector<Dependency> buildQueue;

	void prepareBuildQueue();
	bool wasPackageLoaded(fs::path root_) const;
	PackagePtr findPackageByRoot(fs::path root_) const;
	void loadDependencies(Package &pkg_);
	static bool compareDependency(Dependency const& left_, Dependency const& right_);
public:
	void generate(Package & package_);
};

}


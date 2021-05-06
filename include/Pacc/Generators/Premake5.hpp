#pragma once

#include PACC_PCH

#include <Pacc/Package.hpp>

namespace gen
{

class Premake5
{
public:
	struct ProjectDep
	{
		Project* 	project;
		Dependency* dep;
		
		bool operator==(ProjectDep const& rhs_) const
		{
			return project == rhs_.project && dep == rhs_.dep;
		}
	};

	using DepQueueStep 		= std::vector<ProjectDep>;
	using DepQueue 			= std::vector<DepQueueStep>;
	using PendingDeps 		= DepQueueStep;

	void generate(Package & package_);

private:

	std::vector<PackagePtr> loadedPackages;
	PendingDeps 			pendingDeps;

	void prepareBuildQueue();
	bool wasPackageLoaded(fs::path root_) const;
	PackagePtr findPackageByRoot(fs::path root_) const;
	void loadDependencies(Package &pkg_);
	static bool compareDependency(Dependency const& left_, Dependency const& right_);

	DepQueueStep collectReadyDeps(DepQueue const& ready_, PendingDeps & pending_);
	DepQueue setupConfigQueue();
};

}


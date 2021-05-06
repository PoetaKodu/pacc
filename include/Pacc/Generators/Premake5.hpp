#pragma once

#include PACC_PCH

#include <Pacc/Package.hpp>

namespace gen
{

class ConfigQueue
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

	std::vector<PackagePtr> loadedPackages;
	PendingDeps 			pendingDeps;

	void loadDependencies(Package &pkg_);
	DepQueue setupConfigQueue();
	void prepareBuildQueue();
private:
	bool wasPackageLoaded(fs::path root_) const;
	PackagePtr findPackageByRoot(fs::path root_) const;

	DepQueueStep collectReadyDeps(DepQueue const& ready_, PendingDeps & pending_);
};

class Premake5
{
public:
	void generate(Package & package_);
	
};

}


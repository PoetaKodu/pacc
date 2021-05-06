#pragma once

#include PACC_PCH

#include <Pacc/Package.hpp>

class BuildQueueBuilder
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


	void recursiveLoad(Package &pkg_);
	DepQueue const& setupConfigQueue();

private:

	bool 			isPackageLoaded(fs::path root_) const;
	PackagePtr 		findPackageByRoot(fs::path root_) const;
	DepQueueStep 	collectReadyDependencies(DepQueue const& ready_, PendingDeps & pending_);

	std::vector<PackagePtr> loadedPackages;
	PendingDeps 			pendingDeps;
	DepQueue 				queue; // prepared queue
};

template <typename T>
T& targetByAccessType(AccessSplit<T> & accessSplit_, AccessType type_);
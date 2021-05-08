#pragma once

#include PACC_PCH

#include <Pacc/PackageSystem/Package.hpp>

class BuildQueueBuilder
{
public:

	/// <summary>
	///		Project with it's dependency packed into struct with 2 fields.
	///		Created instead of using `std::pair` because of custom field naming.
	/// </summary>
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


	/// <summary>
	/// 	Builds the queue.
	/// 	Load root package first with `recursiveLoad`
	/// </summary>
	/// <returns>cref to built queue.</returns>
	DepQueue const& setup();

	/// <summary>Recursively loads package dependencies</summary>
	/// <param name="pkg_">The package to get dependencies from.</param>
	void recursiveLoad(Package &pkg_);

private:

	DepQueueStep 			collectReadyDependencies(DepQueue const& ready_, PendingDeps & pending_);

	bool 					isPackageLoaded(fs::path root_) const;
	PackagePtr 				findPackageByRoot(fs::path root_) const;

	std::vector<PackagePtr> loadedPackages;
	PendingDeps 			pendingDeps;
	DepQueue 				queue; // prepared queue
};

template <typename T>
T& targetByAccessType(AccessSplit<T> & accessSplit_, AccessType type_);
#pragma once

#include PACC_PCH

#include <Pacc/PackageSystem/Package.hpp>

class PaccApp;

class BuildQueueBuilder
{
public:
	PaccApp& app;

	explicit BuildQueueBuilder(PaccApp& app_)
		: app(app_)
	{
	}

	/// <summary>
	///		Project with it's dependency packed into struct with 2 fields.
	///		Created instead of using `std::pair` because of custom field naming.
	/// </summary>
	struct ProjectDep
	{
		Project* 		project;
		Dependency* 	dep;

		bool operator==(ProjectDep const& rhs_) const
		{
			return project == rhs_.project
				&& dep == rhs_.dep;
		}
	};

	using DepQueueStep 		= Vec<ProjectDep>;
	using DepQueue 			= Vec<DepQueueStep>;
	using PendingDeps 		= DepQueueStep;

	DepQueue const& 		getQueue() const { return queue; }

	/// <summary>
	/// 	Builds the queue.
	/// 	Load root package first with `recursiveLoad`
	/// </summary>
	/// <returns>cref to built queue.</returns>
	DepQueue const& setup();

	/// <summary>Recursively loads package dependencies</summary>
	/// <param name="pkg_">The package to get dependencies from.</param>
	void recursiveLoad(Package &pkg_);

	/// <summary></summary>
	void performConfigurationMerging();

private:

	DepQueueStep 			collectReadyDependencies(DepQueue const& ready_, PendingDeps & pending_);

	bool 					isPackageLoaded(fs::path root_) const;
	PackagePtr 				findPackageByRoot(fs::path root_) const;

	Vec<PackagePtr> loadedPackages;
	PendingDeps 			pendingDeps;
	DepQueue 				queue; // prepared queue
};



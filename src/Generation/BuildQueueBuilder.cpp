#include "include/Pacc/PaccPCH.hpp"

#include <Pacc/Generation/BuildQueueBuilder.hpp>
#include <Pacc/Generation/OutputFormatter.hpp>
#include <Pacc/System/Filesystem.hpp>
#include <Pacc/System/Environment.hpp>
#include <Pacc/App/App.hpp>


using DepQueue		= BuildQueueBuilder::DepQueue;
using DepQueueStep	= BuildQueueBuilder::DepQueueStep;

///////////////////////////////////////////////////////////////
// Private functions (forward declaration)
///////////////////////////////////////////////////////////////
bool wasDependencyQueued(Dependency const& dep, DepQueue const& readyQueue_);
bool projectHasPendingDependencies(Project const& project, DepQueue const& readyQueue_);
bool packageHasPendingDependencies(PackageDependency & dep, DepQueue const& readyQueue_);
bool selfPackageHasPendingDependencies(SelfDependency & dep, DepQueue const& readyQueue_);


///////////////////////////////////////////////////////////////
// Public functions:
///////////////////////////////////////////////////////////////

void BuildQueueBuilder::performConfigurationMerging()
{
	auto const& q = this->getQueue();

	// fmt::print("Configuration steps: {}\n", q.size());
	// size_t stepCounter = 0;
	for(auto & step : q)
	{
		// fmt::print("Step {}: [ ", stepCounter++);
		// size_t depCounter = 0;
		for(auto & depInfo : step)
		{
			// if (depCounter++ != 0)
			// 	fmt::print(", ");
			// fmt::print("\"{}\"", dep.project->name);

			Project& mergeTarget = *(depInfo.project);

			AccessType mergeMode = depInfo.dep->accessType;

			if (depInfo.dep->isSelf())
			{
				auto &selfDependency = depInfo.dep->self();

				Project const& depProj = selfDependency.package->requireProject(selfDependency.depProjName);

				mergeTarget.inheritConfigurationFrom(*selfDependency.package, depProj, mergeMode );
			}
			else if (depInfo.dep->isPackage())
			{
				auto& packageDependency = depInfo.dep->package();
				// Should be loaded by now
				auto& depReferencedPkg = *packageDependency.package;

				for (auto const & depProjName : packageDependency.projects)
				{
					Project const& depProj = depReferencedPkg.requireProject(depProjName);

					mergeTarget.inheritConfigurationFrom(depReferencedPkg, depProj, mergeMode );
				}
			}
		}
		// fmt::print(" ]\n", stepCounter++);
	}
}


/////////////////////////////////////////////////
void BuildQueueBuilder::recursiveLoad(Package & pkg_)
{
	const std::array<AccessType, 3> methodsLoop = {
			AccessType::Private,
			AccessType::Public,
			AccessType::Interface
		};
	size_t methodIdx = 0;

	for(auto & p : pkg_.projects)
	{
		Vec<Configuration*> configs;
		configs.reserve(1 + p.premakeFilters.size());
		configs.push_back(&p);
		for(auto& [key, value] : p.premakeFilters)
		{
			configs.push_back(&value);
		}

		for(Configuration* cfg : configs)
		{
			methodIdx = 0;
			for(auto* access : getAccesses(cfg->dependencies.self))
			{
				for(auto& dep : *access)
				{
					dep.accessType = methodsLoop[methodIdx];

					switch(dep.type())
					{
					case Dependency::Raw:
					{
						auto& rawDep = dep.raw();
						// fmt::print("Added raw dependency \"{}\"\n", rawDep);

						auto& target = targetByAccessType(cfg->linkedLibraries.computed, dep.accessType);
						// TODO: improve this:
						target.push_back( rawDep );
						break;
					}
					case Dependency::Self:
					{
						pendingDeps.push_back( { &p, &dep } );
						break;
					}
					case Dependency::Package:
					{
						pendingDeps.push_back( { &p, &dep } );

						auto& pkgDep = dep.package();

						PackagePtr pkgPtr;

						// Load dependency (and bind it to shared pointer)
						{

							UPtr<Package> pkg;
							try {
								pkg = app.loadPackageByName(pkgDep.packageName, pkgDep.version, &pkg, "auto");
							}
							catch(PaccException &)
							{
								// This means that the package was loaded, but does not meet version requirements.
								if (pkg && !pkg->name.empty())
								{
									throw PaccException("Could not load package \"{}\". Version \"{}\" is incompatible with requirement \"{}\"",
											pkgDep.packageName, pkg->version.toString(), pkgDep.version.toString()
										)
										.withHelp(
											"Consider installing version of the package that meets requirements.\n"
											"You can list available package versions with \"pacc lsver [package_name]\"\n"
											"To install package at a specific version, use \"pacc install [package_name]@[version]\""
										);
								}
								else
									throw; // Rethrow exception
							}

							if (this->isPackageLoaded(pkg->root))
								continue; // ignore package, was loaded yet

							// if (pkgDep.version.empty())
							// 	fmt::print("Loaded dependency \"{}\"\n", pkgDep.packageName);
							// else
							// 	fmt::print("Loaded dependency \"{}\"@\"{}\"\n", pkgDep.packageName, pkgDep.version);

							pkgPtr = std::move(pkg);
						}

						// Assign loaded package:
						pkgDep.package = pkgPtr;

						// Insert in sorted order:
						{
							auto it = rg::upper_bound( loadedPackages, pkgPtr->root, {}, &Package::root );

							loadedPackages.insert(it, pkgPtr);
						}
						this->recursiveLoad(*pkgPtr);

						break;
					}
					}
				}

				methodIdx++;
			}
		}
	}
}

/////////////////////////////////////////////////
DepQueue const& BuildQueueBuilder::setup()
{
	std::size_t totalDeps 		= pendingDeps.size();
	std::size_t totalCollected 	= 0;

	while(totalCollected < totalDeps)
	{
		DepQueueStep step = this->collectReadyDependencies(queue, pendingDeps);

		// Could not collect any?
		if (step.empty())
			throw PaccException("cyclic dependency detected");

		totalCollected += step.size();

		queue.push_back( std::move(step) );
	}

	return queue;
}

/////////////////////////////////////////////////
DepQueueStep BuildQueueBuilder::collectReadyDependencies(DepQueue const& ready_, PendingDeps & pending_)
{
	PendingDeps newPending;
	newPending.reserve(pending_.size());

	DepQueueStep nextStep;

	for(auto& dep : pending_)
	{
		bool ready = false;
		if (dep.dep->isPackage())
		{
			ready = !packageHasPendingDependencies(dep.dep->package(), ready_);
		}
		else if (dep.dep->isSelf())
		{
			ready = !selfPackageHasPendingDependencies(dep.dep->self(), ready_);
		}

		if (!ready)
			newPending.push_back(dep);
		else
			nextStep.push_back(dep);
	}

	pending_ = std::move(newPending);
	return nextStep;
}

/////////////////////////////////////////////////
PackagePtr BuildQueueBuilder::findPackageByRoot(fs::path root_) const
{
	auto it = rg::lower_bound( loadedPackages, root_, {}, &Package::root );

	if (it != loadedPackages.end() && (*it)->root == root_)
		return *it;

	return nullptr;
}


/////////////////////////////////////////////////
bool BuildQueueBuilder::isPackageLoaded(fs::path root_) const
{
	return rg::binary_search( loadedPackages, root_, {}, &Package::root );
}


///////////////////////////////////////////////////////////////
// Private functions:
///////////////////////////////////////////////////////////////


/////////////////////////////////////////////////
bool wasDependencyQueued(Dependency const& dep, DepQueue const& readyQueue_)
{
	for (auto const& step : readyQueue_)
	{
		// TODO: Can be done better (binary search on sorted range)
		auto it = rg::find(step, &dep, &BuildQueueBuilder::ProjectDep::dep );

		if (it != step.end())
			return true;
	}

	return false;
}

/////////////////////////////////////////////////
bool projectHasPendingDependencies(Project const& project, DepQueue const& readyQueue_)
{
	auto selfDepsAcc = getAccesses(project.dependencies.self);

	for (auto access : selfDepsAcc)
	{
		for(auto& selfDep : *access)
		{
			if (!selfDep.isPackage() && !selfDep.isSelf())
				continue;

			if (!wasDependencyQueued(selfDep, readyQueue_))
				return true;
		}
	}
	return false;
}

/////////////////////////////////////////////////
bool packageHasPendingDependencies(PackageDependency & dep, DepQueue const& readyQueue_)
{
	auto& packagePtr = dep.package;

	for (auto const& projectName : dep.projects)
	{
		// Find pointer to project:
		auto& project = packagePtr->requireProject(projectName);

		if (projectHasPendingDependencies(project, readyQueue_))
			return true;
	}

	return false;
}

/////////////////////////////////////////////////
bool selfPackageHasPendingDependencies(SelfDependency & dep, DepQueue const& readyQueue_)
{
	auto& packagePtr = dep.package;

	// Find pointer to project:
	auto& project = packagePtr->requireProject(dep.depProjName);

	if (projectHasPendingDependencies(project, readyQueue_))
		return true;

	return false;
}

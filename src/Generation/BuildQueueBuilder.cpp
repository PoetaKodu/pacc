#include PACC_PCH

#include <Pacc/Generation/BuildQueueBuilder.hpp>
#include <Pacc/Generation/OutputFormatter.hpp>
#include <Pacc/System/Filesystem.hpp>
#include <Pacc/System/Environment.hpp>


///////////////////////////////////////////////////////////////
// Private functions (forward declaration)
///////////////////////////////////////////////////////////////
bool wasDependencyQueued(Dependency const& dep, BuildQueueBuilder::DepQueue const& readyQueue_);
bool projectHasPendingDependencies(Project const& project, BuildQueueBuilder::DepQueue const& readyQueue_);
bool packageHasPendingDependencies(PackageDependency & dep, BuildQueueBuilder::DepQueue const& readyQueue_);
bool selfPackageHasPendingDependencies(SelfDependency & dep, BuildQueueBuilder::DepQueue const& readyQueue_);


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
		methodIdx = 0;
		for(auto* access : getAccesses(p.dependencies.self))
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

					auto& target = targetByAccessType(p.linkedLibraries.computed, dep.accessType);
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
							pkg = Package::loadByName(pkgDep.packageName, pkgDep.version, &pkg);
						} 
						catch(PaccException &)
						{
							// This means that the package was loaded, but does not meet version requirements.
							if (pkg && !pkg->name.empty())
							{
								throw PaccException("Could not load package \"{}\". Version \"{}\" is incompatible with requirement \"{}\"",
										pkgDep.packageName, pkg->version.toString(), pkgDep.version.toString()
									)
									.withHelp("Consider installing version of the package that meets requirements.\nYou can list available package versions with \"pacc lsver [package_name]\"\nTo install package at a specific version, use \"pacc install [package_name]@[version]\"\n");
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
						auto it = std::upper_bound(
								loadedPackages.begin(), loadedPackages.end(),
								pkgPtr->root,
								[](fs::path const& inserted, auto const& e) { return e->root < inserted; }
							);

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

/////////////////////////////////////////////////
BuildQueueBuilder::DepQueue const& BuildQueueBuilder::setup()
{
	std::size_t totalDeps 		= pendingDeps.size();
	std::size_t totalCollected 	= 0;

	while(totalCollected < totalDeps)
	{
		DepQueueStep step = this->collectReadyDependencies(queue, pendingDeps);
		
		// Could not collect any?
		if (step.empty())
			throw std::runtime_error("cyclic dependency detected");

		totalCollected += step.size();

		queue.push_back( std::move(step) );
	}

	return queue;
}

/////////////////////////////////////////////////
BuildQueueBuilder::DepQueueStep BuildQueueBuilder::collectReadyDependencies(DepQueue const& ready_, PendingDeps & pending_)
{
	PendingDeps newPending;
	newPending.reserve(pending_.size());

	DepQueueStep nextStep;

	for(auto depIt = pending_.begin(); depIt != pending_.end(); ++depIt)
	{
		auto& dep = *depIt;

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
	auto it = std::lower_bound(
			loadedPackages.begin(), loadedPackages.end(),
			root_,
			[](auto const& e, fs::path const& inserted) { return e->root < inserted; }
		);

	if (it != loadedPackages.end() && (*it)->root == root_)
		return *it;

	return nullptr;
}


/////////////////////////////////////////////////
bool BuildQueueBuilder::isPackageLoaded(fs::path root_) const
{
	auto it = std::lower_bound(
			loadedPackages.begin(), loadedPackages.end(),
			root_,
			[](auto const& e, fs::path const& inserted) { return e->root < inserted; }
		);

	if (it != loadedPackages.end() && (*it)->root == root_)
		return true;

	return false;
}


///////////////////////////////////////////////////////////////
// Private functions:
///////////////////////////////////////////////////////////////


/////////////////////////////////////////////////
bool wasDependencyQueued(Dependency const& dep, BuildQueueBuilder::DepQueue const& readyQueue_)
{
	auto compareQueuedDep =
		[&](BuildQueueBuilder::ProjectDep const& readyDep)
		{
			return readyDep.dep == &dep;
		};

	for (auto const& step : readyQueue_)
	{
		// TODO: Can be done better (binary search on sorted range)
		auto it = std::find_if(step.cbegin(), step.cend(), compareQueuedDep);
		
		if (it != step.end())
		{
			return true;
		}
	}

	return false;
}

/////////////////////////////////////////////////
bool projectHasPendingDependencies(Project const& project, BuildQueueBuilder::DepQueue const& readyQueue_)
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
bool packageHasPendingDependencies(PackageDependency & dep, BuildQueueBuilder::DepQueue const& readyQueue_)
{
	auto& packagePtr = dep.package;

	for (auto const& projectName : dep.projects)
	{
		// Find pointer to project:
		auto project = packagePtr->findProject(projectName);

		if (projectHasPendingDependencies(*project, readyQueue_))
			return true;	
	}

	return false;
}

/////////////////////////////////////////////////
bool selfPackageHasPendingDependencies(SelfDependency & dep, BuildQueueBuilder::DepQueue const& readyQueue_)
{
	auto& packagePtr = dep.package;

	// Find pointer to project:
	auto project = packagePtr->findProject(dep.depProjName);

	if (projectHasPendingDependencies(*project, readyQueue_))
		return true;	

	return false;
}

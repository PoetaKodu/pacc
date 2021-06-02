#include PACC_PCH

#include <Pacc/App/App.hpp>

#include <Pacc/App/Help.hpp>
#include <Pacc/System/Environment.hpp>
#include <Pacc/App/Errors.hpp>
#include <Pacc/PackageSystem/Package.hpp>
#include <Pacc/PackageSystem/Version.hpp>
#include <Pacc/System/Filesystem.hpp>
#include <Pacc/System/Process.hpp>
#include <Pacc/Generation/BuildQueueBuilder.hpp>
#include <Pacc/Generation/Logs.hpp>
#include <Pacc/Readers/General.hpp>
#include <Pacc/Readers/JsonReader.hpp>
#include <Pacc/Helpers/Formatting.hpp>
#include <Pacc/Helpers/Exceptions.hpp>
#include <Pacc/Helpers/String.hpp>


#include <Pacc/Toolchains/General.hpp>


///////////////////////////////////////////////////
std::vector<PackageDependency> PaccApp::collectMissingDependencies(Package const & pkg_)
{
	std::vector<PackageDependency> result;

	for (auto const& proj : pkg_.projects)
	{
		for (auto* acc : getAccesses(proj.dependencies.self))
		{
			for (auto const& dep : *acc)
			{
				if (dep.isPackage())
				{
					auto pkgDep = dep.package();

					try {
						Package::loadByName(pkgDep.packageName); // just try to load
					}
					catch (...) {
						result.push_back(std::move(pkgDep));
					}
					// Ignore.
				}
			}
		}
	}

	return result;
}

///////////////////////////////////////////////////
void PaccApp::downloadPackage(fs::path const &target_, DownloadLocation const& loc_)
{
	constexpr int GitListInvalidUrl = 128;
	constexpr auto CouldNotLoad 		= "Could not load package \"{0}\"";
	constexpr auto DependencyNotFound 	= "Could not find remote repository \"{}\"";
	constexpr auto CouldNotClone 		= "Could not clone remote repository \"{0}\", error code: {1}";

	constexpr auto ListRemoteCommand 	= "git ls-remote \"{}\"";
	constexpr auto BranchParam 			= "\"--branch={}\" "; // Notice the space at the end
	constexpr auto CloneCommand 		= "git clone --depth=1 {2}\"{0}\" \"{1}\""; // 2 -> branch param

	// Ensure dependency is valid:
	if (loc_.repository.empty()
		|| loc_.platform == DownloadLocation::Unknown
		|| (loc_.userName.empty() && loc_.platform != DownloadLocation::OfficialRepo) )
	{
		throw PaccException(CouldNotLoad, loc_.repository);
	}

	std::string cloneLink = loc_.getGitLink();
	
	// Ensure repository exists and is available:
	{
		auto command 		= fmt::format(ListRemoteCommand, cloneLink);
		auto process 		= ChildProcess{ command, "", ch::seconds{2} };
		auto listExitStatus	= process.runSync();

		if (listExitStatus.value_or(GitListInvalidUrl) != 0)
		{
			throw PaccException(DependencyNotFound, cloneLink);
		}
	}

	// Clone the repository
	{
		std::string branchParam;
		
		std::string branch = loc_.getBranch();
		if (!branch.empty())
			branchParam = fmt::format(BranchParam, branch);

		auto cloneCommand 		= fmt::format(CloneCommand, cloneLink, fsx::fwd(target_).string(), branchParam);
		auto cloneExitStatus 	= ChildProcess{ cloneCommand, "", ch::seconds{60} }.runSync();

		if (cloneExitStatus.value_or(1) != 0)
		{
			if (!branch.empty())
			{
				throw PaccException(CouldNotClone, cloneLink, cloneExitStatus.value_or(-1))
					.withHelp("Make sure that the version/branch \"{}\" is correct.\nUse \"pacc lsver {}\" to check available versions.", loc_.branch, loc_.repository);
			}
			else 
				throw PaccException(CouldNotClone, cloneLink, cloneExitStatus.value_or(-1));
		}
	}

	// Remove `.git` folder:
	fs::path gitFolderPath = target_ / ".git";
	if (fs::is_directory(gitFolderPath))
	{
		fsx::makeWritableAll(gitFolderPath);

		fs::remove_all(gitFolderPath);
	}
}

///////////////////////////////////////////////////
void PaccApp::loadPaccConfig()
{
	using fmt::fg, fmt::color;

	fs::path const cfgPath = env::getPaccDataStorageFolder() / "settings.json";

	cfg = PaccConfig::loadOrCreate(cfgPath);

	auto tcs = detectAllToolchains();

	if (cfg.ensureValidToolchains(tcs))
	{
		fmt::print(fg(color::yellow) | fmt::emphasis::bold,
				"Warning: detected new toolchains, resetting the default one\n"
			);
	}
}

///////////////////////////////////////////////////
bool PaccApp::containsSwitch(std::string_view switch_) const
{
	// Arg 0 -> program name with path
	// Arg 1 -> action name
	// Start at 2
	for(size_t i = 2; i < args.size(); ++i)
	{
		if (startsWith(args[i], switch_))
			return true;
	}

	return false;
}

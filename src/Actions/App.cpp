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
void PaccApp::initPackage()
{
	using fmt::fg, fmt::color;

	auto cwd = fs::current_path();

	fs::path target 		= cwd;
	std::string targetName 	= cwd.stem().string();

	if (args.size() > 2 && args[2] != ".")
	{
		targetName = args[2];

		if (fs::path(targetName).is_relative())
			target /= targetName;
		else
			target = targetName;

		if (fs::exists(target / "cpackage.json"))
		{
			throw PaccException("Folder \"{}\" already contains cpackage.json!", targetName);
		}
	}

	std::cout << "Initializing package \"" << target.stem().string() << "\"" << std::endl;
	std::cout << "Do you want to create \"cpackage.json\" file (Y/N): ";

	std::string response;
	std::getline(std::cin, response);

	if (response[0] != 'y' && response[0] != 'Y')
	{
		std::cout << "Action aborted." << std::endl;
		return;
	}

	fs::create_directories(target);

	std::ofstream(target / "cpackage.json") << fmt::format(
R"PKG({{
	"$schema": "https://raw.githubusercontent.com/PoetaKodu/pacc/main/res/cpackage.schema.json",
	
	"name": "{}",
	"projects": [
		{{
			"name": "MyProject",
			"type": "app",
			"language": "C++17",
			"files": "src/*.cpp"
		}}
	]
}})PKG", target.stem().u8string());

	fmt::print(fg(color::lime_green),
			"\"cpackage.json\" has been created.\n"
			"Happy development!"
		);
}

///////////////////////////////////////////////////
void PaccApp::linkPackage()
{
	auto pkg = Package::load();

	fs::path appData = env::getPaccDataStorageFolder();

	fs::path packagesDir 	= appData / "packages";
	fs::path targetSymlink 	= packagesDir / pkg->name;

	fs::create_directories(packagesDir);

	if (fs::exists(targetSymlink))
	{
		if (fs::is_symlink(targetSymlink))
		{
			throw PaccException(
					"Package \"{}\" is already linked to {}.\n",
					pkg->name,
					fs::read_symlink(targetSymlink).string()
				)
				.withHelp("If you want to update the link, use \"pacc unlink\" first.");
		}
		else
		{
			throw PaccException(
					"Package \"{}\" is already installed in users environment.\n",
					pkg->name
				)
				.withHelp("If you want to link current package, uninstall existing one with \"pacc uninstall\" first.");
		}
	}
	else
	{
		fs::create_directory_symlink(fs::current_path(), targetSymlink);
		fmt::print("Package \"{}\" has been linked inside the user environment.", pkg->name);
	}
}

///////////////////////////////////////////////////
void PaccApp::toolchains()
{
	auto const &tcs = cfg.detectedToolchains;

	if (args.size() >= 3)
	{
		int tcIdx = -1;

		try {
			tcIdx = std::stoi(std::string(args[2]));
		}
		catch(...) {}

		if (tcIdx < 0 || tcIdx >= int(tcs.size()))
		{
			throw PaccException("Invalid toolchain id \"{:.10}\"", args[2])
				.withHelp("Use \"pacc tc\" to list available toolchains.");
		}

		fmt::print("Changed selected toolchain to {} (\"{}\", version \"{}\")",
				tcIdx,
				tcs[tcIdx]->prettyName,	
				tcs[tcIdx]->version	
			);

		cfg.updateSelectedToolchain(tcIdx);
	}
	else
	{
		// Display toolchains
		std::cout << "TOOLCHAINS:\n";
			
		if (!tcs.empty())
		{
			using fmt::fg, fmt::color;
			using namespace fmt::literals;

			auto const& style = fmt_args::s();

			size_t maxNameLen = 20;
			for(auto& tc : tcs)
				maxNameLen = std::max(maxNameLen, tc->prettyName.length());
			fmt::print("    ID{0:4}{Name}{0:{NameLen}}{Version}\n{0:-^{NumDashes}}\n",
					"",
					FMT_INLINE_ARG("Name", 		fg(color::lime_green), "Name"),
					FMT_INLINE_ARG("Version", 	fg(color::aqua), "Version"),

					"NameLen"_a 	= maxNameLen,
					"NumDashes"_a 	= maxNameLen + 20 + 4
				);

			// TODO: add user configuration with specified default toolchain.
			int idx = 0;
			for (auto& tc : tcs)
			{
				bool selected = (idx == cfg.selectedToolchain);
				auto style = selected ? fmt::emphasis::bold : fmt::text_style{};
				fmt::print(style, "{:>6}    {:{NameLen}}    {:10}\n",
						fmt::format("{} #{}", selected ? '>' : ' ', idx),
						tc->prettyName,
						tc->version,

						"NameLen"_a = maxNameLen
					);
				idx++;
			}
		}
		else
		{
			fmt::print("\tNo toolchains detected :(\n");
		}
	}
}

///////////////////////////////////////////////////
void PaccApp::unlinkPackage()
{
	std::string pkgName;
	if (args.size() > 2)
		pkgName = args[2];

	if (pkgName.empty())
	{
		auto pkg = Package::load();
		pkgName = pkg->name;
	}

	fs::path storage = env::getPaccDataStorageFolder();
	fs::path symlinkPath = storage / "packages" / pkgName;
	if (fs::is_symlink(symlinkPath))
	{
		fs::remove(symlinkPath);
		fmt::print("Package \"{}\" has been unlinked from the user environment.", pkgName);
	}
	else
	{
		throw PaccException(
				"Package \"{}\" is not linked within user environment.\n",
				pkgName
			).withHelp("If you want to link current package, use \"pacc link\" first.");
	}	
}

///////////////////////////////////////////////////
void PaccApp::runPackageStartupProject()
{
	auto pkg = Package::load();

	if (pkg->projects.empty())
		throw PaccException("Package \"{}\" does not contain any projects.", pkg->name);

	auto settings = this->determineBuildSettingsFromArgs();

	Project const* project = nullptr;

	std::string targetName;
	// Try to get target name from args (ignore build settings --target flag, too complex)
	if (args.size() >= 3 && args[2][0] != '-') // not a switch
		targetName = args[2];

	if (targetName.empty())
		targetName = pkg->startupProject;

	if (targetName.empty())
	{
		for(auto const &proj : pkg->projects)
		{
			if (proj.type == "app")
			{
				project = &pkg->projects[0];
				break;
			}
		}
		if (!project)
			throw PaccException("Package \"{}\" does not contain any runnable projects.", pkg->name);
	}
	else
	{
		project = pkg->findProject(targetName);
		if (!project)
			throw PaccException("Package \"{}\" does not contain project with name \"{}\".", pkg->name, targetName);
	}

	if (project->type != "app")
	{
		throw PaccException("Cannot run project \"{}\", because it's not an application (type: \"{}\", expected: \"app\").", project->name, project->type)
			.withHelp("If the package contains other application projects, use \"pacc run [project_name]\"\n");
	}

	fs::path outputFile = fsx::fwd(pkg->predictRealOutputFolder(*project, settings) / project->name);

	#ifdef PACC_SYSTEM_WINDOWS
	outputFile += ".exe";
	#endif

	if (!fs::exists(outputFile))
		throw PaccException("Could not find project \"{}\" binary.", project->name)
			.withHelp("Use \"pacc build\" command first and make sure it succeeded.");

	auto before = ch::steady_clock::now();

	auto exitStatus = ChildProcess{outputFile.string(), "", std::nullopt, true}.runSync();

	auto dur = ch::duration_cast< ch::duration<double> >(ch::steady_clock::now() - before);

	fmt::print("\nProgram ended after {:.2f}s with {} exit status.", dur.count(), exitStatus.value_or(1));
}


///////////////////////////////////////////////////
void setupBuildQueue(Package & pkg, BuildQueueBuilder& depQueue)
{
	depQueue.recursiveLoad(pkg);
	depQueue.setup();
	depQueue.performConfigurationMerging();
}

///////////////////////////////////////////////////
void PaccApp::generate()
{
	auto pkg = Package::load();
	BuildQueueBuilder depQueue;
	setupBuildQueue(*pkg, depQueue);
	this->createPremake5Generator().generate(*pkg);
}

///////////////////////////////////////////////////
gen::Premake5 PaccApp::createPremake5Generator()
{
	gen::Premake5 gen;
	gen.compileCommands = this->containsSwitch("--compile-commands") || this->containsSwitch("-cc");
	return gen;
}

///////////////////////////////////////////////////
void handleBuildResult(ChildProcess::ExitCode exitStatus_)
{
	using fmt::fg, fmt::color;

	auto lastLogNotice = []{
		fmt::print(fg(color::light_sky_blue) | fmt::emphasis::bold, "\nNote: you can print last log using \"pacc log --last\".\n");
	};

	if (exitStatus_.has_value())
	{
		if (exitStatus_.value() == 0)
		{
			fmt::print(fg(color::green), "success\n");
			fmt::print(fmt::fg(fmt::color::lime_green), "Build succeeded.\n");
			lastLogNotice();
			return;
		}
		else
			fmt::print(fg(color::dark_red), "failure\n");
	}
	else
		fmt::printErr(fg(color::red), "timeout\n");

	fmt::printErr(fg(color::red) | fmt::emphasis::bold, "Build failed.\n");
	lastLogNotice();
}

///////////////////////////////////////////////////
void PaccApp::ensureProjectsAreBuilt(Package const& pkg_, std::vector<std::string> const& projectNames_, BuildSettings const& settings_)
{
	fs::path rootFolder = pkg_.root.parent_path();

	fs::path prevWorkingDir = fs::current_path();

	using fmt::fg, fmt::color;
	for (auto const& projName : projectNames_)
	{
		Project const* p = pkg_.findProject(projName);
		// Build only static and shared libs
		if (p->type != "static lib" && p->type != "shared lib")
			continue;


		fs::path binaryPath = rootFolder / "bin" / settings_.platformName / settings_.configName;
		#ifdef PACC_SYSTEM_WINDOWS
			binaryPath /= (projName + ".lib");	
		#else
			binaryPath /= projName + ".a"; // TODO: determine it by used toolchain
		#endif
		if (!fs::exists(binaryPath))
		{
			fmt::print("Building dependency project \"{}\" from package \"{}\".\n", projName, pkg_.name);

			fs::current_path(rootFolder);
			try {
				this->buildSpecifiedPackage(pkg_, *cfg.currentToolchain(), settings_);
			}
			catch(...)
			{
				// Ensure right working directory
				fs::current_path(prevWorkingDir);
				throw;
			}
			fs::current_path(prevWorkingDir);
				
		}
	}
}

///////////////////////////////////////////////////
void PaccApp::ensureDependenciesBuilt(Package const& pkg_, BuildQueueBuilder const &depQueue_, BuildSettings const& settings_)
{
	using fmt::fg, fmt::color;

	size_t numDeps = 0;
	for(auto const& stage : depQueue_.getQueue())
	{
		for(auto const& dep : stage)
		{
			if (dep.dep->isPackage())
				++numDeps;
		}
	}

	if (numDeps == 0)
		return;

	fmt::print(fg(color::light_gray), "Ensuring {} dependencies are built.\n", numDeps);

	for(auto const& stage : depQueue_.getQueue())
	{
		for (auto const& dep : stage)
		{
			if (dep.dep->isPackage())
			{
				auto const& pkgDep = dep.dep->package();

				ensureProjectsAreBuilt(*pkgDep.package, pkgDep.projects, settings_);
			}
		}
	}
}

///////////////////////////////////////////////////
void PaccApp::buildPackage()
{
	if (auto tc = cfg.currentToolchain())
	{
		auto settings = this->determineBuildSettingsFromArgs();
		
		auto pkg = Package::load(fs::current_path());

		BuildQueueBuilder depQueue;
		setupBuildQueue(*pkg, depQueue);
		ensureDependenciesBuilt(*pkg, depQueue, settings);

		this->buildSpecifiedPackage( *pkg, *tc, settings );
	}
	else
	{
		throw PaccException("No toolchain selected.")
			.withHelp("Use \"pacc tc <toolchain id>\" to select toolchain.");
	}
}

///////////////////////////////////////////////////
void PaccApp::buildSpecifiedPackage(Package const & pkg_, Toolchain& toolchain_, BuildSettings const& settings_)
{
	this->createPremake5Generator().generate(pkg_);

	// Run premake:
	gen::runPremakeGeneration(toolchain_.premakeToolchainType());

	// Run build toolchain
	int verbosityLevel = (this->containsSwitch("--verbose")) ? 1 : 0;
	handleBuildResult( toolchain_.run(pkg_, settings_, verbosityLevel) );
}

///////////////////////////////////////////////////
void PaccApp::install()
{
	using fmt::fg, fmt::color;

	bool global = false;
	if (this->containsSwitch("-g") || this->containsSwitch("--global"))
		global = true;

	fs::path targetPath;
	if (global)
		targetPath = env::requirePaccDataStorageFolder() / "packages";
	else
		targetPath = "pacc_packages";

	 // TODO: improve this
	if (args.size() >= (global ? 4 : 3))
	{
		std::string packageTemplate(args[2]);

		auto loc = DownloadLocation::parse( packageTemplate );

		fs::path targetPackagePath = targetPath / loc.repository;
		if (fs::is_directory(targetPackagePath) || fs::is_symlink(targetPackagePath))
		{
			throw PaccException("Package \"{0}\" is already installed{1}.", loc.repository, global ? " globally" : "")
				.withHelp("Uninstall the package with \"pacc uninstall {0}{1}\"\n", loc.repository, global ? " --global" : "");
		}

		this->downloadPackage(targetPackagePath, loc);

		fmt::print(fg(color::lime_green), "Installed package \"{}\".\n", loc.repository);
	}
	else
	{
		if (global)
			throw PaccException("Missing argument: package name")
				.withHelp("Use \"pacc install [package_name] --global\"");

		auto pkg = Package::load();

		auto deps = this->collectMissingDependencies(*pkg);

		if (deps.empty())
		{
			fmt::print("No packages to install.\n");
			return;
		}

		size_t numInstalled = 0;

		try {
			for (auto const& dep : deps)
			{
				auto loc = DownloadLocation::parse( dep.downloadLocation );
				if (loc.platform == DownloadLocation::Unknown)
				{
					throw PaccException("Missing package \"{}\" with no download location specified, or the location is wrong.", dep.packageName)
						.withHelp(
								"Provide \"from\" for the package. Use following syntax:\n{}\n",
								help::DependencySyntax
							);
				}

				fs::path targetPackagePath = targetPath / dep.packageName;
				if (fs::is_directory(targetPackagePath) || fs::is_symlink(targetPackagePath))
				{
					throw PaccException("Package folder \"{}\" is already used.", targetPackagePath.filename().string())
						.withHelp("Remove the folder.\n");
				}

				this->downloadPackage(targetPackagePath, loc);

				// TODO: download package dependencies

				// TODO: run install script on package.


				++numInstalled;
			}
		}
		catch(...)
		{
			fmt::printErr(fg(color::red), "Installed {} / {} packages.\n", numInstalled, deps.size());
			throw;
		}

		fmt::print(fg(color::lime_green), "Installed {} / {} packages.\n", numInstalled, deps.size());
	}
}

///////////////////////////////////////////////////
void PaccApp::uninstall()
{
	using fmt::fg, fmt::color;

	bool global = false;
	if (this->containsSwitch("-g") || this->containsSwitch("--global"))
		global = true;

	fs::path targetPath;
	if (global)
		targetPath = env::requirePaccDataStorageFolder() / "packages";
	else
		targetPath = "pacc_packages";

	 // TODO: improve this
	if (args.size() >= (global ? 4 : 3))
	{
		std::string packageName(args[2]);

		fs::path packagePath = targetPath / packageName;

		if (fs::is_symlink(packagePath) && global)
		{
			fsx::makeWritableAll(packagePath);
			fs::remove(packagePath);
			fmt::print("Package \"{}\" has been unlinked from the user environment.", packageName);
		}
		else if (fs::is_directory(packagePath))
		{
			fsx::makeWritableAll(packagePath);
			fs::remove_all(packagePath);
			fmt::print(fg(color::lime_green), "Uninstalled package \"{}\".\n", packageName);
		}
		else
		{
			if (fs::exists(packagePath))
			{
				throw PaccException("Invalid type of package \"{}\".", packageName)
					.withHelp("Directory or symlink required\n");
			}
			else
			{
				throw PaccException("Package \"{0}\" is not installed{1}.", packageName, global ? " globally" : "");
			}
		}
	}
	else
		throw PaccException("Missing argument: package name")
			.withHelp("Use \"pacc uninstall [package_name]\"");
}

///////////////////////////////////////////////////
void PaccApp::listVersions()
{
	using fmt::fg, fmt::color;
	auto boldBlue 	= fmt::emphasis::bold | fg(color::light_sky_blue);
	auto boldYellow = fmt::emphasis::bold | fg(color::yellow);

	constexpr auto DependencyNotFound 	= "Could not find remote repository \"{}\"";

	constexpr auto ListRemoteCommand 	= "git ls-remote --tags --refs --sort=v:refname \"{}\"";


	if (args.size() < 3)
	{
		throw PaccException("Missing argument: package name")
			.withHelp(
					"Use following syntax: \"pacc lsver [package_name]\", where \"package_name\" is \n{}\n",
					help::DependencySyntax
				);
	}

	std::string dependencyTemplate(args[2]);

	auto loc = DownloadLocation::parse(dependencyTemplate);

	std::string repoLink = loc.getGitLink();

	std::string output;
	// List versions
	{
		auto command 		= fmt::format(ListRemoteCommand, repoLink);
		auto process 		= ChildProcess{ command, "", ch::seconds{2} };
		auto listExitStatus	= process.runSync();

		if (listExitStatus.value_or(1) != 0)
		{
			throw PaccException(DependencyNotFound, repoLink);
		}

		output = std::move(process.out.stdOut);
	}

	auto versions = PackageVersions::parse(output).sort();

	{
		VersionReq req;
		if (args.size() >= 4 && args[3][0] != '-')
		{
			req = VersionReq::fromString( std::string(args[3]) );
			versions = versions.filter(req);
		}
		else
		{
			fmt::print(boldBlue, "Note: you filter compatible versions, f.e.: \"pacc lsver fmt ^7.1\"\n\n" );
		}

		bool showTags = this->containsSwitch("--tags");

		fmt::print("PACKAGE {}:\n", showTags ? "TAGS" : "VERSIONS");
		if (showTags)
		{
			fmt::print(boldBlue, "Note: viewing real git tags. Remove \"--tags\" param to see version syntax.\n" );
		}

		// Print compatible:
		{
			int NumPerRow = showTags ? 3 : 4;
			size_t numVer = versions.confirmed.size();

			fmt::print("Compatible {}:\n", showTags ? "tags (prefix \"pacc-\" is required)" : "versions");
			if (numVer > 0)
			{
				for(size_t i = 0; i < numVer; i += NumPerRow)
				{
					for (int j = 0; (j < NumPerRow && i + j < numVer); ++j)
					{
						auto const& v = versions.confirmed[i + j];
						if (showTags)
							fmt::print("{:>16}", v.first);
						else
							fmt::print("{:>12}", v.second.toString());
					}
					fmt::print("\n");
				}
			}
			else
				fmt::print( boldBlue, "    None\n" );
		}

		// Print rest:
		if (this->containsSwitch("--all"))
		{
			constexpr int NumPerRow = 4;
			size_t numVer = versions.rest.size();

			fmt::print("Unconfirmed {}:\n", showTags ? "tags" : "versions (prefix \"!\" is required)");
			if (numVer > 0)
			{
				for(size_t i = 0; i < numVer; i += 4)
				{
					for (int j = 0; (j < 4 && i + j < numVer); ++j)
					{
						auto const& v = versions.rest[i + j];
						if (showTags)
							fmt::print("{:>12}", v.first);
						else
							fmt::print("{:>12}", "!" + v.second.toString());
					}
					fmt::print("\n");
				}
			}
			else
				fmt::print( boldBlue, "    None\n" );

		}
	}

}

///////////////////////////////////////////////////
void PaccApp::logs()
{
	// Print latest
	if (containsSwitch("--last"))
	{
		auto logs = getSortedBuildLogs(1);
		if (logs.empty())
		{
			fmt::print("No build logs found.\n");
		}
		else
		{
			std::string content = readFileContents(logs[0]);
			fmt::print("{}\n", content);
		}
	}
	else
	{
		size_t amount = 10;

		if (args.size() >= 3)
		{
			try {
				amount = std::stol( std::string(args[2]) );
			}
			catch(...) {}
		}
		else
		{
			using fmt::fg, fmt::color;
			fmt::print(
					fg(color::light_sky_blue) | fmt::emphasis::bold,
					"Note: you can set viewed log limit, f.e.: \"pacc log 3\" (default: 10)\n"
				);
		}

		fmt::print("LATEST BUILD LOGS:\n");

		auto logs = getSortedBuildLogs(amount);
		if (logs.empty())
		{
			fmt::print("    No build logs found.\n");
		}
		else
		{
			for(int i = 0; i < logs.size(); ++i)
			{
				fmt::print("{:>4}: {}\n", fmt::format("#{}", i), logs[i].filename().string());
			}
		}
	}

}

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
void PaccApp::cleanupLogs(size_t maxLogs_) const
{
	auto logs = getSortedBuildLogs();

	if (logs.size() > maxLogs_)
	{
		for(size_t i = maxLogs_; i < logs.size(); ++i)
		{
			fs::remove(logs[i]);
		}
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
void PaccApp::displayHelp(bool abbrev_)
{
	auto programName = fs::u8path(args[0]).stem();

	auto const& style = fmt_args::s();

	// Introduction:
	fmt::print( "pacc v{} - a C++ package manager.\n\n"
				"{USAGE}: {} [action] <params>\n\n",
				PaccApp::PaccVersion,
				programName.string(),

				FMT_INLINE_ARG("USAGE", style.Yellow, "USAGE")
			);

	// 
	if (abbrev_)
	{
		fmt::print("Use \"{} help\" for more information\n", programName.string());
	}
	else
	{
		// Display actions
		std::cout << "ACTIONS\n";
					
		for (auto action : help::actions)
		{
			fmt::print("\t{:16}{}\n", action.first, action.second);
		}
		std::cout << std::endl;
	}
}

///////////////////////////////////////////////////
BuildSettings PaccApp::determineBuildSettingsFromArgs() const
{
	using SwitchNames = std::vector<std::string>;
	static const SwitchNames target 		= { "--target", "-t" };
	static const SwitchNames platforms 		= { "--platform", "--plat", "-p" };
	static const SwitchNames configurations = { "--configuration", "--config", "--cfg", "-c" };
	
	auto parseSwitch = [](std::string_view arg, SwitchNames const& switches, std::string& val)
		{
			for(auto sw : switches)
			{
				if (parseArgSwitch(arg, sw, val))
					return true;
			}
			return false;
		};

	BuildSettings result;

	// Arg 0 -> program name with path
	// Arg 1 -> action name
	// Start at 2
	for(size_t i = 2; i < args.size(); ++i)
	{
		std::string switchVal;
		if (parseSwitch(args[i], platforms, switchVal))
		{
			result.platformName = std::move(switchVal);
		}
		else if (parseSwitch(args[i], configurations, switchVal))
		{
			result.configName = std::move(switchVal);
		}
		else if (parseSwitch(args[i], target, switchVal))
		{
			result.targetName = std::move(switchVal);
		}
	}

	return result;
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

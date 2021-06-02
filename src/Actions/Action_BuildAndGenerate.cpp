#include PACC_PCH

#include <Pacc/App/App.hpp>

#include <Pacc/System/Process.hpp>

///////////////////////////////////////////////////
void setupBuildQueue(Package & pkg, BuildQueueBuilder& depQueue)
{
	depQueue.recursiveLoad(pkg);
	depQueue.setup();
	depQueue.performConfigurationMerging();
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
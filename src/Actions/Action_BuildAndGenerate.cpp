#include "include/Pacc/PaccPCH.hpp"

#include <Pacc/App/App.hpp>

#include <Pacc/Readers/General.hpp>
#include <Pacc/System/Process.hpp>

///////////////////////////////////////////////////
void setupBuildQueue(Package & pkg, BuildQueueBuilder& depQueue)
{
	depQueue.recursiveLoad(pkg);
	depQueue.setup();
	depQueue.performConfigurationMerging();
}

///////////////////////////////////////////////////
void handleBuildResult(ChildProcess::ExitCode exitStatus_, bool isDependency_)
{
	using fmt::fg, fmt::color;

	auto lastLogNotice = [&]{
		if (!isDependency_)
			fmt::print(fg(color::light_sky_blue) | fmt::emphasis::bold, "\nNote: you can print last log using \"pacc log --last\".\n");
	};

	if (exitStatus_.has_value())
	{
		if (exitStatus_.value() == 0)
		{
			fmt::print(fg(color::green), "success\n");
			if (isDependency_)
				fmt::print(fmt::fg(fmt::color::green), "Dependency build succeeded.\n");
			else
				fmt::print(fmt::fg(fmt::color::green), "Build succeeded.\n");
			lastLogNotice();


			return;
		}
		else
			fmt::print(fg(color::dark_red), "failure\n");
	}
	else
		fmt::printErr(fg(color::red), "timeout\n");

	if (isDependency_)
		fmt::printErr(fg(color::red) | fmt::emphasis::bold, "Dependency build failed.\n");
	else
		fmt::printErr(fg(color::red) | fmt::emphasis::bold, "Build failed.\n");

	lastLogNotice();
}


///////////////////////////////////////////////////
void PaccApp::generate()
{
	auto pkg = Package::load();

	auto depQueue = BuildQueueBuilder{*this};
	setupBuildQueue(*pkg, depQueue);
	this->createPremake5Generator().generate(*pkg);
}

///////////////////////////////////////////////////
auto PaccApp::createPremake5Generator() -> gen::Premake5
{
	auto gen = gen::Premake5();
	gen.compileCommands = settings.isFlagSet("--compile-commands");
	return gen;
}

///////////////////////////////////////////////////
auto PaccApp::runPremakeGeneration(StringView toolchainName_) -> void
{
	using fmt::fg, fmt::color;

	fmt::print(fg(color::gray), "Running Premake5... ");

	auto command = fmt::format("\"{}\" {}", this->getPremake5Path().string(), toolchainName_);

	auto exitStatus = ChildProcess{command, "", ch::seconds{30}}.runSync();

	if (exitStatus.has_value())
	{
		if (exitStatus.value() == 0)
			fmt::print(fg(color::green), "success\n");
	}
	else
		fmt::printErr(fg(color::red), "timeout\n");

	if (int es = exitStatus.value_or(1))
	{
		throw PaccException("Failed to generate project files (Premake5 exit code: {})", es);
	}
}

///////////////////////////////////////////////////
void PaccApp::ensureProjectsAreBuilt(Package& pkg_, Vec<String> const& projectNames_, BuildSettings const& settings_)
{
	auto rootFolder = pkg_.root.parent_path();
	auto prevWorkingDir = fs::current_path();

	using fmt::fg, fmt::color;
	for (auto const& projName : projectNames_)
	{
		auto p = pkg_.findProject(projName);
		// Build only static and shared libs
		if (p->type != Project::StaticLib && p->type != Project::SharedLib)
			continue;

		auto binaryPath = rootFolder / "bin" / settings_.platformName / settings_.configName;

		auto tc = *cfg.currentToolchain();

		if (tc.type() == Toolchain::GNUMake)
			binaryPath /= "lib" + projName + ".a";
		else if (tc.type() == Toolchain::MSVC)
			binaryPath /= (projName + ".lib");
		// else: error

		fmt::print("Binaries for project {} are located at {}\n", p->name, pkg_.getAbsoluteArtifactFilePath(*p, settings_).string());

		if (!fs::exists(binaryPath) && !fs::exists(pkg_.getAbsoluteArtifactFilePath(*p, settings_)))
		{
			fmt::print("Building dependency project \"{}\" from package \"{}\".\n", projName, pkg_.name);

			fs::current_path(rootFolder);
			try {
				this->buildSpecifiedPackage(pkg_, *cfg.currentToolchain(), settings_, true);
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
void PaccApp::ensureDependenciesBuilt(Package& pkg_, BuildQueueBuilder const &depQueue_, BuildSettings const& settings_)
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
		auto settings	= this->determineBuildSettingsFromArgs();
		auto pkg		= this->loadPackage(fs::current_path(), "auto");

		auto depQueue = BuildQueueBuilder{*this};
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
void PaccApp::buildSpecifiedPackage(Package& pkg_, Toolchain& toolchain_, BuildSettings const& settings_, bool isDependency_)
{
	this->execPackageEvent(pkg_, "build");

	auto builder = pkg_.builder ? pkg_.builder : defaultPackageBuilder;

	// Run build toolchain
	auto verbosityLevel = int(settings.isFlagSet("--verbose") ? 1 : 0);
	handleBuildResult( builder->run(pkg_, toolchain_, settings_, verbosityLevel), isDependency_ );

	this->execPackageEvent(pkg_, "post:build");
}

///////////////////////////////////////////////////
auto PaccApp::determineBuildSettingsFromArgs() const -> BuildSettings
{
	using SwitchNames = Vec<String>;
	static const auto cores 			= SwitchNames{ "--cores" };
	static const auto target 			= SwitchNames{ "--target", "-t" };
	static const auto platforms 		= SwitchNames{ "--platform", "--plat", "-p" };
	static const auto configurations 	= SwitchNames{ "--configuration", "--config", "--cfg", "-c" };

	auto parseSwitch = [](StringView arg, SwitchNames const& switches, String& val)
		{
			for(auto sw : switches)
			{
				if (parseArgSwitch(arg, sw, val))
					return true;
			}
			return false;
		};

	auto result = BuildSettings();

	// Arg 0 -> program name with path
	// Arg 1 -> action name
	// Start at 2

	// cores
	{
		auto& arg = *settings.flags.at("--cores");
		if (arg.isSet())
		{
			result.cores = convertTo<int>(String(arg.value));
		}
	}

	// platform name
	{
		auto& arg = *settings.flags.at("--platform");
		if (arg.isSet())
		{
			result.platformName = String(arg.value);
		}
	}

	// configuration name
	{
		auto& arg = *settings.flags.at("--configuration");
		if (arg.isSet())
		{
			result.configName = String(arg.value);
		}
	}

	// target name
	{
		auto& arg = *settings.flags.at("--target");
		if (arg.isSet())
		{
			result.targetName = String(arg.value);
		}
	}


	return result;
}

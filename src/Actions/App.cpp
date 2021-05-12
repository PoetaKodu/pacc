#include PACC_PCH

#include <Pacc/App/App.hpp>

#include <Pacc/App/Help.hpp>
#include <Pacc/System/Environment.hpp>
#include <Pacc/App/Errors.hpp>
#include <Pacc/PackageSystem/Package.hpp>
#include <Pacc/System/Filesystem.hpp>
#include <Pacc/System/Process.hpp>
#include <Pacc/Generation/Premake5.hpp>
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
	Package pkg = Package::load();

	fs::path appData = env::getPaccDataStorageFolder();

	fs::path packagesDir 	= appData / "packages";
	fs::path targetSymlink 	= packagesDir / pkg.name;

	fs::create_directories(packagesDir);

	if (fs::exists(targetSymlink))
	{
		if (fs::is_symlink(targetSymlink))
		{
			throw PaccException(
					"Package \"{}\" is already linked to {}.\n",
					pkg.name,
					fs::read_symlink(targetSymlink).string()
				)
				.withHelp("If you want to update the link, use \"pacc unlink\" first.");
		}
		else
		{
			throw PaccException(
					"Package \"{}\" is already installed in users environment.\n",
					pkg.name
				)
				.withHelp("If you want to link current package, uninstall existing one with \"pacc uninstall\" first.");
		}
	}
	else
	{
		fs::create_directory_symlink(fs::current_path(), targetSymlink);
		fmt::print("Package \"{}\" has been linked inside the user environment.", pkg.name);
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
		Package pkg = Package::load();
		pkgName = pkg.name;
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
	Package pkg = Package::load();

	if (pkg.projects.empty())
		throw PaccException("Package \"{}\" does not contain any projects.", pkg.name);

	auto settings = this->determineBuildSettingsFromArgs();
	auto const& project = pkg.projects[0];
	fs::path outputFile = fsx::fwd(pkg.predictRealOutputFolder(project, settings) / project.name);

	#ifdef PACC_SYSTEM_WINDOWS
	outputFile += ".exe";
	#endif

	if (!fs::exists(outputFile))
		throw PaccException("Could not find startup project \"{}\" binary.", project.name)
			.withHelp("Use \"pacc build\" command first and make sure it succeeded.");

	auto before = ch::steady_clock::now();

	auto exitStatus = ChildProcess{outputFile.string(), "", std::nullopt, true}.runSync();

	auto dur = ch::duration_cast< ch::duration<double> >(ch::steady_clock::now() - before);

	fmt::print("\nProgram ended after {:.2f}s with {} exit status.", dur.count(), exitStatus.value_or(1));
}


///////////////////////////////////////////////////
void generatePremakeFiles(Package & pkg)
{
	gen::Premake5 g;
	g.generate(pkg);
}

///////////////////////////////////////////////////
Package PaccApp::generate()
{
	Package pkg = Package::load();

	generatePremakeFiles(pkg);

	return pkg;
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
void PaccApp::buildPackage()
{
	Package pkg = generate();

	if (auto tc = cfg.currentToolchain())
	{
		// Run premake:
		gen::runPremakeGeneration(tc->premakeToolchainType());

		// Run build toolchain
		auto settings = this->determineBuildSettingsFromArgs();
		int verbosityLevel = (this->containsSwitch("--verbose")) ? 1 : 0;
		handleBuildResult( tc->run(pkg, settings, verbosityLevel) );
	}
	else
	{
		throw PaccException("No toolchain selected.")
			.withHelp("Use \"pacc tc <toolchain id>\" to select toolchain.");
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
				PaccApp::Version,
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
			fmt::print("\t{:12}{}\n", action.first, action.second);
		}
		std::cout << std::endl;
	}
}

///////////////////////////////////////////////////
BuildSettings PaccApp::determineBuildSettingsFromArgs() const
{
	using SwitchNames = std::vector<std::string>;
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

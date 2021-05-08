#include PACC_PCH

#include <Pacc/Actions/Actions.hpp>

#include <Pacc/Actions/Help.hpp>
#include <Pacc/System/Environment.hpp>
#include <Pacc/Actions/Errors.hpp>
#include <Pacc/PackageSystem/Package.hpp>
#include <Pacc/System/Filesystem.hpp>
#include <Pacc/System/Process.hpp>
#include <Pacc/Generation/Premake5.hpp>
#include <Pacc/Readers/General.hpp>
#include <Pacc/Readers/JsonReader.hpp>
#include <Pacc/Helpers/Formatting.hpp>
#include <Pacc/Helpers/Exceptions.hpp>


#include <Pacc/Toolchains/General.hpp>
#include <Pacc/Toolchains/MSVC.hpp>
#include <Pacc/Toolchains/GNUMake.hpp>

namespace actions
{

// Used by build command:
void 				generateProjectFiles();
void 				buildProjects(Package const &pkg_);

///////////////////////////////////////////////////
void initPackage()
{
	auto cwd = fs::current_path();


	std::cout << "Initializing package \"" << cwd.stem().string() << "\"" << std::endl;
	std::cout << "Do you want to create \"cpackage.json\" file (Y/N): ";

	std::string response;
	std::getline(std::cin, response);

	if (response[0] != 'y' && response[0] != 'Y')
	{
		std::cout << "Action aborted." << std::endl;
		return;
	}

	std::ofstream("cpackage.json") <<
R"PKG({
	"name": "MyWorkspace",
	"projects": [
		{
			"name": "MyProject",
			"type": "app",
			"language": "C++17",
			"files": "src/*.cpp"
		}
	]
})PKG";

	std::cout 	<< "\"cpackage.json\" has been created.\n"
				<< "Happy development!" << std::endl;
}

///////////////////////////////////////////////////
void linkPackage(ProgramArgs const& args_)
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
void toolchains(ProgramArgs const& args_)
{
	
	std::vector<Toolchain const*> tcs;

	auto appendToolchains = [](auto &to, auto const& from)
		{
			to.reserve(from.size());
			for(auto const& tc : from)
				to.push_back(&tc);
		};

	// Detect toolchains:

	// MSVC:
	#ifdef PACC_SYSTEM_WINDOWS
		auto msvcTcs 		= MSVCToolchain::detect();
		appendToolchains(tcs, msvcTcs);
	#endif

	// GNU Make:
	auto gnuMakeTcs 	= GNUMakeToolchain::detect();
	appendToolchains(tcs, gnuMakeTcs);


	// Display actions
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
		constexpr int selectedIdx = 0;
		int idx = 0;
		for (auto& tc : tcs)
		{
			bool selected = (idx == selectedIdx);
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

///////////////////////////////////////////////////
void unlinkPackage(ProgramArgs const& args_)
{
	std::string pkgName;
	if (args_.size() > 2)
		pkgName = args_[2];

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
void runPackageStartupProject(ProgramArgs const& args_)
{
	Package pkg = Package::load();

	if (pkg.projects.empty())
		throw PaccException("Package \"{}\" does not contain any projects.", pkg.name);

	auto const& project = pkg.projects[0];
	fs::path outputFile = fsx::fwd(pkg.predictRealOutputFolder(project) / project.name);

	#ifdef PACC_SYSTEM_WINDOWS
	outputFile += ".exe";
	#endif

	if (!fs::exists(outputFile))
		throw PaccException("Could not find startup project \"{}\" binary.", project.name)
			.withHelp("Use \"pacc build\" command first and make sure it succeeded.");

	auto before = ch::steady_clock::now();

	auto exitStatus = ChildProcess{outputFile.string(), "", -1, true}.runSync();

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
Package generate(ProgramArgs const& args_)
{
	Package pkg = Package::load();

	generatePremakeFiles(pkg);

	return pkg;
}

///////////////////////////////////////////////////
void buildPackage(ProgramArgs const& args_)
{
	Package pkg = generate(args_);

	// Run premake:
	generateProjectFiles();

	// Run msbuild
	buildProjects(pkg);
}

///////////////////////////////////////////////////
void generateProjectFiles()
{
	using fmt::fg, fmt::color;

	fmt::print(fg(color::gray), "Running Premake5... ");

	auto exitStatus = ChildProcess{"premake5 vs2019", "", 10}.runSync();

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
void buildProjects(Package const& pkg_)
{
	using fmt::fg, fmt::color;
	// TODO: add ability to run other build systems.
	// TODO: right now MSBuild has to be in PATH variable. Add ability to find msbuild.

	fmt::print(fg(color::gray), "Running MSBuild... ");

	std::string_view params[] = {
		"/m",
		"/property:Configuration=Debug",
		"/property:Platform=x64",
		// Ask msbuild to generate full paths for file names.
		"/property:GenerateFullPaths=true",
		"/t:build"
	};

	std::string buildCommand = fmt::format("msbuild {0}.sln", pkg_.name);
	for(auto p : params)
		buildCommand += fmt::format(" \"{}\"", p);

	auto exitStatus = ChildProcess{buildCommand, "build", 30}.runSync();

	if (exitStatus.has_value())
	{
		if (exitStatus.value() == 0)
		{
			fmt::print(fg(color::green), "success\n");
			fmt::print(fmt::fg(fmt::color::lime_green), "Build succeeded.\n");
		}
	}
	else
		fmt::printErr(fg(color::red), "timeout\n");

	if (exitStatus.value_or(1) != 0)
		throw std::runtime_error("Build failed.");
}

///////////////////////////////////////////////////
void displayHelp(ProgramArgs const& args_, bool abbrev_)
{
	auto programName = fs::u8path(args_[0]).stem();

	auto const& style = fmt_args::s();

	// Introduction:
	fmt::print( "A C++ package manager.\n\n"
				"{USAGE}: {} [action] <params>\n\n",
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





}
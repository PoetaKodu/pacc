#include <Blocc/Main.hpp>

#include <Blocc/Help.hpp>
#include <Blocc/Errors.hpp>
#include <Blocc/Package.hpp>
#include <Blocc/Generators/Premake5.hpp>
#include <Blocc/Readers/General.hpp>
#include <Blocc/Readers/JSONReader.hpp>
#include <fmt/format.h>
#undef UNICODE
#include <tiny-process-lib/process.hpp>
#define UNICODE

#include <iostream>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <chrono>
#include <thread>
#include <optional>

namespace fs 	= std::filesystem;
namespace proc 	= TinyProcessLib;
namespace ch 	= std::chrono;
namespace tt	= std::this_thread;

////////////////////////////////////
// Forward declarations
////////////////////////////////////

void 				handleArgs(ProgramArgs const & args_);

////////////////
// Commands:

// help
void 				displayHelp(ProgramArgs const& args_, bool full_);
// link
void 				linkPackage(ProgramArgs const& args_);
// unlink
void 				unlinkPackage(ProgramArgs const& args_);
// generate
Package 			generate(ProgramArgs const& args_);
// build
void 				buildPackage(ProgramArgs const& args_);
// init
void 				initPackage();

/////////////////////
// Helper functions:
Package 			loadPackage();
Package 			fromJSON(std::string const& packageContent_);
std::optional<int> 	runChildProcessSync(std::string const& command_, std::string cwd = "", int timeOutSecs = -1);

// Used by build command:
void 				generateProjectFiles();
void 				buildProjects(Package const &pkg_);

fs::path 			getBloccDataStorageFolder();
fs::path 			requireBloccDataStorageFolder();


///////////////////////////////////////////////////
int main(int argc, char *argv[])
{
	ProgramArgs args{ argv, argv + argc };

	try {
		handleArgs(args);
	}
	catch(std::exception & exc)
	{
		std::cerr 	<< "An error occurred. Details:\n" << exc.what();

		return 1;
	}
	catch(...)
	{
		std::cerr 	<< "An error occurred. No details available."
					<< "\nPlease refer to https://github.com/PoetaKodu/blocc/issues"
					<< std::endl;

		return 1;
	}
}

///////////////////////////////////////////////////
void handleArgs(ProgramArgs const& args_)
{
	if (args_.size() < 2)
	{
		displayHelp(args_, true);
	}
	else
	{
		auto action = args_[1];

		if (action == "help")
		{
			displayHelp(args_, false);
		}
		else if (action == "init")
		{
			initPackage();
		}
		else if (action == "generate")
		{
			generate(args_);
		}
		else if (action == "build")
		{
			buildPackage(args_);
		}
		else if (action == "link")
		{
			linkPackage(args_);
		}
		else if (action == "unlink")
		{
			unlinkPackage(args_);	
		}
		else
		{
			auto programName = fs::u8path(args_[0]).stem();

			std::cerr 	<< "Error:\tunsupported action \"" << action
						<< "\".\n\tUse \"" << programName << " help\" to list available actions."
						<< std::endl;
		}
	}
}


///////////////////////////////////////////////////
void displayHelp(ProgramArgs const& args_, bool abbrev_)
{
	auto programName = fs::u8path(args_[0]).stem();

	// Introduction:
	std::cout 	<< "A C++ package manager.\n\n"
			 	<< "USAGE: " << programName.string() << " [action] <params>\n\n";

	// 
	if (abbrev_)
	{
		std::cout 	<< "Use \"" << programName.string() << " help\" for more information"
					<< std::endl;
	}
	else
	{
		// Display actions
		std::cout << "ACTIONS\n";
					
		for (auto action : help::actions)
		{
			std::cout << "\t" << action.first << "\t\t" << action.second << "\n";
		}
		std::cout << std::endl;
	}
}

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
	"$schema": "https://raw.githubusercontent.com/PoetaKodu/blocc/main/res/cpackage.schema.json",
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
	Package pkg = loadPackage();

	fs::path appData = getBloccDataStorageFolder();

	fs::path packagesDir 	= appData / "packages";
	fs::path targetSymlink 	= packagesDir / pkg.name;

	fs::create_directories(packagesDir);

	if (fs::exists(targetSymlink))
	{
		if (fs::is_symlink(targetSymlink))
		{
			throw std::runtime_error(fmt::format(
					"Package \"{}\" is already linked to {}.\n"
					"If you want to update the link, use \"blocc unlink\" first.",
					pkg.name,
					fs::read_symlink(targetSymlink).string()
				));
		}
		else
		{
			throw std::runtime_error(fmt::format(
					"Package \"{}\" is already installed in users environment.\n"
					"If you want to link current package, uninstall existing one with \"blocc uninstall\" first.",
					pkg.name
				));
		}
	}
	else
	{
		fs::create_directory_symlink(fs::current_path(), targetSymlink);
		fmt::print("Package \"{}\" has been linked inside the user environment.", pkg.name);
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
		Package pkg = loadPackage();
		pkgName = pkg.name;
	}

	fs::path storage = getBloccDataStorageFolder();
	fs::path symlinkPath = storage / "packages" / pkgName;
	if (fs::is_symlink(symlinkPath))
	{
		fs::remove(symlinkPath);
		fmt::print("Package \"{}\" has been unlinked from the user environment.", pkgName);
	}
	else
	{
		throw std::runtime_error(fmt::format(
				"Package \"{}\" is not linked within user environment.\n"
				"If you want to link current package, use \"blocc link\" first.",
				pkgName
			));
	}	
}

///////////////////////////////////////////////////
void generatePremakeFiles(Package const& pkg)
{
	gen::Premake5 g;
	g.generate(pkg);
}

///////////////////////////////////////////////////
Package generate(ProgramArgs const& args_)
{
	Package pkg = loadPackage();

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
Package loadPackage()
{
	constexpr std::string_view PackageJSON 	= "cpackage.json";
	constexpr std::string_view PackageLUA 	= "cpackage.lua";

	auto cwd = fs::current_path();

	enum class PackageFileSource
	{
		JSON,
		LuaScript
	};
	PackageFileSource pkgSrcFile;
	

	// Detect package file
	if (fs::exists(cwd / PackageLUA)) // LuaScript has higher priority
	{
		pkgSrcFile = PackageFileSource::LuaScript;
	}
	else if (fs::exists(cwd / PackageJSON))
	{
		pkgSrcFile = PackageFileSource::JSON;
	}
	else
		throw std::exception(errors::NoPackageSourceFile.data());
	

	Package pkg;

	// Decide what to do:
	switch(pkgSrcFile)
	{
	case PackageFileSource::JSON:
	{
		std::cout << "Loading \"" << PackageJSON << "\" file\n";\

		pkg = reader::fromJSON(reader::readFileContents(PackageJSON));
		break;
	}
	case PackageFileSource::LuaScript:
	{
		std::cout << "Loading \"" << PackageLUA << "\" file\n";

		// TODO: implement this.
		std::cout << "This function is not implemented yet." << std::endl;
		break;
	}
	}
	return pkg;
}

///////////////////////////////////////////////////
void generateProjectFiles()
{
	std::cout << "Running Premake5 build (VS2019) config" << std::endl;

	auto exitStatus = runChildProcessSync("premake5 vs2019", "", 10);

	if (exitStatus.has_value())
		std::cout << "Premake5 finished with exit code " << exitStatus.value() << std::endl;
	else
		std::cerr << "Premake5 generation was aborted (reason: timeout)" << std::endl;

	if (exitStatus.value_or(1) != 0)
		throw std::runtime_error("Failed to generate project files");
}

///////////////////////////////////////////////////
void buildProjects(Package const& pkg_)
{
	// TODO: add ability to run other build systems.
	// TODO: right now MSBuild has to be in PATH variable. Add ability to find msbuild.

	std::cout << "Running MSBuild" << std::endl;

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

	auto exitStatus = runChildProcessSync(buildCommand, "build", 30);

	if (exitStatus.has_value())
		std::cout << "MSBuild finished with exit code " << exitStatus.value() << std::endl;
	else
		std::cerr << "MSBuild build was aborted (reason: timeout)" << std::endl;

	if (exitStatus.value_or(1) != 0)
		throw std::runtime_error("Failed to build package projects");
}

///////////////////////////////////////////////////
std::optional<int> runChildProcessSync(std::string const& command_, std::string cwd, int timeOutSecs)
{
	auto prevCwd = fs::current_path();
	if (cwd != "")
		fs::current_path(cwd); 
		
	proc::Process proc(command_, "",
		// Handle stdout:
		[](const char *bytes, size_t n)
		{
			std::cout << std::string(bytes, n);
			if(bytes[n - 1] != '\n')
				std::cout << std::endl;
		},
		// Handle stderr:
		[](const char *bytes, size_t n)
		{
			std::cerr << std::string(bytes, n);
			if(bytes[n - 1] != '\n')
				std::cout << std::endl;
		}
	);

	bool killed 	= false;
	int exitStatus 	= 1;
	int runTime 	= 0;
	while(!proc.try_get_exit_status(exitStatus))
	{
		if (timeOutSecs != -1)
		{
			if (runTime++ > timeOutSecs * 10)
			{
				proc.kill();
				killed = true;
				break;
			}
		}

		tt::sleep_for(ch::milliseconds{100});
	}

	if (cwd != "")
		fs::current_path(prevCwd); 

	if (killed)
		return std::nullopt;

	return exitStatus;
}

///////////////////////////////////////////////////
fs::path getBloccDataStorageFolder()
{
	fs::path appData;
	#ifdef BLOCC_SYSTEM_WINDOWS
		appData = std::getenv("APPDATA");
	#else
		appData = std::getenv("USER");
	#endif
	appData /= "blocc";
	return appData;
}

///////////////////////////////////////////////////
fs::path requireBloccDataStorageFolder()
{
	fs::path const storage = getBloccDataStorageFolder();
	fs::create_directories(storage);
	return storage;
}
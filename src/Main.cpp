#include <CppPkg/Main.hpp>

#include <CppPkg/Help.hpp>

#include <iostream>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

// Forward declarations
void handleArgs(ProgramArgs const & args_);
void displayHelp(ProgramArgs const& args_, bool full_);
void initPackage();



///////////////////////////////////////////////////
int main(int argc, char *argv[])
{
	ProgramArgs args{ argv, argv + argc };

	handleArgs(args);
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

void initPackage()
{
	auto cwd = fs::current_path();


	std::cout << "Initializing package \"" << cwd.stem().string() << "\"" << std::endl;
	std::cout << "Do you want to create \"package.lua\" file (Y/N): ";

	std::string response;
	std::getline(std::cin, response);

	if (response[0] != 'y' && response[0] != 'Y')
	{
		std::cout << "Action aborted." << std::endl;
		return;
	}

	std::ofstream("package.lua") <<
R"PKG(
-- Package configuration file.
-- For more info visit https://github.com/PoetaKodu/cpp-pkg/docs/PackageFile/README.md

return {
	name = "",
	type = "app",
	language = "c++17"
	files = "src/*.cpp"
})PKG";

	std::cout 	<< "\"package.lua\" has been created.\n"
				<< "Happy development!" << std::endl;
}

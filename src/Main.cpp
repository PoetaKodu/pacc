#include BLOCC_PCH

#include <Blocc/Main.hpp>

#include <Blocc/Help.hpp>
#include <Blocc/Actions.hpp>

////////////////////////////////////
// Forward declarations
////////////////////////////////////
void handleArgs(ProgramArgs const & args_);


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
		actions::displayHelp(args_, true);
	}
	else
	{
		auto action = args_[1];

		if (action == "help")
		{
			actions::displayHelp(args_, false);
		}
		else if (action == "init")
		{
			actions::initPackage();
		}
		else if (action == "generate")
		{
			actions::generate(args_);
		}
		else if (action == "build")
		{
			actions::buildPackage(args_);
		}
		else if (action == "link")
		{
			actions::linkPackage(args_);
		}
		else if (action == "unlink")
		{
			actions::unlinkPackage(args_);	
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



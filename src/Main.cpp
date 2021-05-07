#include PACC_PCH

#include <Pacc/Main.hpp>

#include <Pacc/Help.hpp>
#include <Pacc/Actions.hpp>
#include <Pacc/Helpers/Exceptions.hpp>
#include <Pacc/Helpers/Formatting.hpp>

////////////////////////////////////
// Forward declarations
////////////////////////////////////
void handleArgs(ProgramArgs const & args_);


///////////////////////////////////////////////////
int main(int argc, char *argv[])
{
	using namespace fmt::literals;

	ProgramArgs args{ argv, argv + argc };


	try {
		handleArgs(args);
	}
	catch(PaccException & exc)
	{
		dumpException(exc);
		
	}
	catch(std::exception & exc)
	{
		dumpException(exc);
		return 1;
	}
	catch(...)
	{
		fmt::printErr(	"{Error}\n"
						"    An unknown error occurred.\n"
						"    No details available\n"
						"    Please refer to https://github.com/PoetaKodu/pacc/issues\n",

						fmt_args::error());

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
		else if (action == "toolchain" || action == "toolchain" || action == "tc")
		{
			actions::toolchains(args_);
		}
		else if (action == "run")
		{
			actions::runPackageStartupProject(args_);
		}
		else
		{
			auto programName = fs::u8path(args_[0]).stem();

			throw PaccException("unsupported action \"{}\"", action)
					.withHelp("Use \"{} help\" to list available actions.", programName.string());
		}
	}
}



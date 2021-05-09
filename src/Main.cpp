#include PACC_PCH

#include <Pacc/Main.hpp>

#include <Pacc/App/Help.hpp>
#include <Pacc/App/App.hpp>
#include <Pacc/Helpers/Exceptions.hpp>
#include <Pacc/Helpers/Formatting.hpp>
#include <Pacc/App/PaccConfig.hpp>
#include <Pacc/System/Environment.hpp>
#include <Pacc/Toolchains/General.hpp>

////////////////////////////////////
// Forward declarations
////////////////////////////////////
void handleArgs(ProgramArgs args_);


///////////////////////////////////////////////////
int main(int argc, char *argv[])
{
	using namespace fmt::literals;

	fmt::enableColors();

	ProgramArgs args{ argv, argv + argc };


	try {
		handleArgs(std::move(args));
	}
	catch(PaccException & exc)
	{
		dumpException(exc);
		return 1;
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
void handleArgs(ProgramArgs args_)
{
	using fmt::color, fmt::fg;

	PaccApp app;
	app.args = std::move(args_);

	if (app.args.size() < 2)
	{
		app.displayHelp(true);
	}
	else
	{
		fs::path const cfgPath = env::getPaccDataStorageFolder() / "settings.json";

		app.cfg = PaccConfig::loadOrCreate(cfgPath);

		auto tcs = detectAllToolchains();

		if (app.cfg.ensureValidToolchains(tcs))
		{
			fmt::print(fg(color::yellow) | fmt::emphasis::bold,
					"Warning: detected new toolchains, resetting the default one\n"
				);
		}

		auto action = app.args[1];

		if (action == "help")
		{
			app.displayHelp(false);
		}
		else if (action == "init")
		{
			app.initPackage();
		}
		else if (action == "generate")
		{
			app.generate();
		}
		else if (action == "build")
		{
			app.buildPackage();
		}
		else if (action == "link")
		{
			app.linkPackage();
		}
		else if (action == "unlink")
		{
			app.unlinkPackage();	
		}
		else if (action == "toolchain" || action == "toolchain" || action == "tc")
		{
			app.toolchains();
		}
		else if (action == "run")
		{
			app.runPackageStartupProject();
		}
		else
		{
			auto programName = fs::u8path(app.args[0]).stem();

			throw PaccException("unsupported action \"{}\"", action)
					.withHelp("Use \"{} help\" to list available actions.", programName.string());
		}
	}
}



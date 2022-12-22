#include "include/Pacc/PaccPCH.hpp"

#include <Pacc/Main.hpp>

#include <Pacc/App/Help.hpp>
#include <Pacc/App/App.hpp>
#include <Pacc/Helpers/Exceptions.hpp>
#include <Pacc/Helpers/Formatting.hpp>
#include <Pacc/Helpers/String.hpp>
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

	auto args = ProgramArgs{ argv, argv + argc };


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
		fmt::printErr(	fmt::runtime("{Error}\n"
						"    An unknown error occurred.\n"
						"    No details available\n"
						"    Please refer to https://github.com/PoetaKodu/pacc/issues\n"),

						fmt_args::error());

		return 1;
	}
}

///////////////////////////////////////////////////
void handleArgs(ProgramArgs args_)
{
	using fmt::color, fmt::fg;

	auto& app = useApp();
	app.initialWorkingDirectory = fs::current_path();
	app.args = std::move(args_);

	// TODO: make configurable
	app.cleanupLogs(200);

	if (app.args.size() < 2)
	{
		app.displayHelp(true);
	}
	else
	{
		using Action = PaccMainAction;

		app.settings = RunSettings::fromArgs(app.args);

		// Initial switch for trivial commands
		switch (app.settings.mainAction)
		{
		case Action::None:
		{
			auto programName = fs::u8path(app.args[0]).stem();

			if (app.settings.actionNameIndex == 0)
			{
				throw PaccException("No action specified")
					.withHelp("Use \"{} help\" to list available actions.", programName.string());
			}

			throw PaccException("Unrecognized action \"{}\"", app.args[app.settings.actionNameIndex])
				.withHelp("Use \"{} help\" to list available actions.", programName.string());

		}
		case Action::Version:
		{
			fmt::print("pacc v{}\n", PaccApp::PaccVersion);
			return;
		}
		case Action::Help:
		{
			app.displayHelp(false);
			return;
		}
		}



		app.setupLua();

		// For non-trivial commands
		switch(app.settings.mainAction)
		{
		case Action::Init:
		{
			app.loadPaccConfig();

			app.initPackage();
			break;
		}
		case Action::Generate:
		{
			app.loadPaccConfig();

			app.generate();
			break;
		}
		case Action::Build:
		{
			app.loadPaccConfig();

			app.buildPackage();
			break;
		}
		case Action::Link:
		{
			app.linkPackage();
			break;
		}
		case Action::Unlink:
		{
			app.unlinkPackage();
			break;
		}
		case Action::Logs:
		{
			app.logs();
			break;
		}
		case Action::Install:
		{
			app.install();
			break;
		}
		case Action::Uninstall:
		{
			app.uninstall();
			break;
		}
		case Action::ListVersions:
		{
			app.listVersions();
			break;
		}
		case Action::ListPackages:
		{
			app.listPackages();
			break;
		}
		case Action::Toolchains:
		{
			app.loadPaccConfig();

			app.toolchains();
			break;
		}
		case Action::Run:
		{
			app.loadPaccConfig();

			app.run();
			break;
		}
		}
	}
}



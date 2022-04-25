#include PACC_PCH

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

	PaccApp app;
	app.args = std::move(args_);

	app.setupLua();

	// TODO: make configurable
	app.cleanupLogs(200);

	if (app.args.size() < 2)
	{
		app.displayHelp(true);
	}
	else
	{
		auto action = toLower(app.args[1]);

		if (action == "version")
		{
			fmt::print("pacc v{}\n", PaccApp::PaccVersion);
		}
		else if (action == "help")
		{
			app.displayHelp(false);
		}
		else if (action == "init")
		{
			app.loadPaccConfig();

			app.initPackage();
		}
		else if (action == "generate")
		{
			app.loadPaccConfig();

			app.generate();
		}
		else if (action == "build")
		{
			app.loadPaccConfig();

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
		else if (action == "logs" || action == "log")
		{
			app.logs();
		}
		else if (action == "install" || action == "i")
		{
			app.install();
		}
		else if (action == "uninstall")
		{
			app.uninstall();
		}
		else if (action == "list-versions" || action == "list-version" || action == "lsver")
		{
			app.listVersions();
		}
		else if (action == "list-packages" || action == "list" || action == "ls")
		{
			app.listPackages();
		}
		else if (action == "toolchains" || action == "toolchain" || action == "tc")
		{
			app.loadPaccConfig();

			app.toolchains();
		}
		else if (action == "run")
		{
			app.loadPaccConfig();

			app.run();
		}
		else if (action == "graph")
		{
			app.visualizeGraph();
		}
		else if (action == "query")
		{
			app.query();
		}
		else
		{
			auto programName = fs::u8path(app.args[0]).stem();

			throw PaccException("unsupported action \"{}\"", action)
					.withHelp("Use \"{} help\" to list available actions.", programName.string());
		}
	}
}



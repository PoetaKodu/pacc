#include PACC_PCH

#include <Pacc/App/App.hpp>
#include <Pacc/App/Help.hpp>

#include <Pacc/Helpers/Formatting.hpp>

///////////////////////////////////////////////////
void PaccApp::displayHelp(bool abbrev_)
{
	auto programName = fs::u8path(args[0]).stem();

	auto const& style = fmt_args::s();

	// Introduction:
	fmt::print( "pacc v{} - a C++ package manager.\n\n"
				"{USAGE}: {} [action] <params>\n\n",
				PaccApp::PaccVersion,
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
			fmt::print("\t{:16}{}\n", action.first, action.second);
		}
		std::cout << std::endl;
	}
}
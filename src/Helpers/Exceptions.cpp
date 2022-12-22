#include "include/Pacc/PaccPCH.hpp"

#include <Pacc/Helpers/Exceptions.hpp>

//////////////////////////////////////////////////
void dumpException(std::exception const& exc_)
{
	fmt::printErr(	fmt::runtime("{Error} "
					"{}\n"),
					exc_.what(),

					fmt_args::error(), fmt_args::details());
}

//////////////////////////////////////////////////
void dumpException(PaccException const& exc_)
{
	auto& help = exc_.help();
	auto& note = exc_.note();

	dumpException(static_cast<std::exception const&>(exc_));

	if (!help.empty())
	{
		fmt::printErr(fmt::runtime("\n{Help} "
						"{}\n"),
						exc_.help(),
						fmt_args::help());
	}

	if (!note.empty())
	{
		fmt::printErr(fmt::runtime("\n{Note} "
						"{}\n"),
						exc_.note(),
						fmt_args::note());
	}
}

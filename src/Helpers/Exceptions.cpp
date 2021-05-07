#include PACC_PCH

#include <Pacc/Helpers/Exceptions.hpp>

//////////////////////////////////////////////////
void dumpException(std::exception const& exc_)
{
	fmt::printErr(	"{Error} "
					"{Details}:\n"
					"    {}\n",
					exc_.what(),

					fmt_args::error(), fmt_args::details());
}

//////////////////////////////////////////////////
void dumpException(PaccException const& exc_)
{
	dumpException(static_cast<std::exception const&>(exc_));
	
	if (!exc_.help().empty())
	{
		fmt::printErr("{Help}\n"
						"    {}",
						exc_.help(),

						fmt_args::help());
	}
}
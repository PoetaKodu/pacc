#include "include/Pacc/PaccPCH.hpp"

#include <Pacc/App/App.hpp>

#include <Pacc/Generation/Logs.hpp>
#include <Pacc/Readers/General.hpp>


///////////////////////////////////////////////////
void PaccApp::logs()
{
	// Print latest
	if (settings.flags.at("--last")->isSet())
	{
		auto logs = getSortedBuildLogs(1);
		if (logs.empty())
		{
			fmt::print("No build logs found.\n");
		}
		else
		{
			String content = readFileContents(logs[0]);
			fmt::print("{}\n", content);
		}
	}
	else
	{
		size_t amount = 10;

		auto amountArgIdx = settings.nthActionArgument(0);
		if (amountArgIdx)
		{
			try {
				amount = std::stol( String(args[*amountArgIdx]) );
			}
			catch(...) {}
		}
		else
		{
			using fmt::fg, fmt::color;
			fmt::print(
					fg(color::light_sky_blue) | fmt::emphasis::bold,
					"Note: you can set viewed log limit, f.e.: \"pacc log 3\" (default: 10)\n"
				);
		}

		fmt::print("LATEST BUILD LOGS:\n");

		auto logs = getSortedBuildLogs(amount);
		if (logs.empty())
		{
			fmt::print("    No build logs found.\n");
		}
		else
		{
			for(int i = 0; i < logs.size(); ++i)
			{
				fmt::print("{:>4}: {}\n", fmt::format("#{}", i), logs[i].filename().string());
			}
		}
	}

}


///////////////////////////////////////////////////
void PaccApp::cleanupLogs(size_t maxLogs_) const
{
	auto logs = getSortedBuildLogs();

	if (logs.size() > maxLogs_)
	{
		for(size_t i = maxLogs_; i < logs.size(); ++i)
		{
			fs::remove(logs[i]);
		}
	}
}

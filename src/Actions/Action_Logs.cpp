#include PACC_PCH

#include <Pacc/App/App.hpp>

#include <Pacc/Generation/Logs.hpp>
#include <Pacc/Readers/General.hpp>


///////////////////////////////////////////////////
void PaccApp::logs()
{
	// Print latest
	if (containsSwitch("--last"))
	{
		auto logs = getSortedBuildLogs(1);
		if (logs.empty())
		{
			fmt::print("No build logs found.\n");
		}
		else
		{
			std::string content = readFileContents(logs[0]);
			fmt::print("{}\n", content);
		}
	}
	else
	{
		size_t amount = 10;

		if (args.size() >= 3)
		{
			try {
				amount = std::stol( std::string(args[2]) );
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
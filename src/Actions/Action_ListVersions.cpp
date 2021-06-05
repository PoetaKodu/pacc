#include PACC_PCH

#include <Pacc/App/App.hpp>

#include <Pacc/App/Help.hpp>
#include <Pacc/System/Process.hpp>


///////////////////////////////////////////////////
void PaccApp::listVersions()
{
	using fmt::fg, fmt::color;
	auto boldBlue 	= fmt::emphasis::bold | fg(color::light_sky_blue);
	auto boldYellow = fmt::emphasis::bold | fg(color::yellow);

	constexpr auto DependencyNotFound 	= "Could not find remote repository \"{}\"";

	constexpr auto ListRemoteCommand 	= "git ls-remote --tags --refs --sort=v:refname \"{}\"";


	if (args.size() < 3)
	{
		throw PaccException("Missing argument: package name")
			.withHelp(
					"Use following syntax: \"pacc lsver [package_name]\", where \"package_name\" is \n{}",
					help::DependencySyntax
				);
	}

	std::string dependencyTemplate(args[2]);

	auto loc = DownloadLocation::parse(dependencyTemplate);

	std::string repoLink = loc.getGitLink();

	std::string output;
	// List versions
	{
		auto command 		= fmt::format(ListRemoteCommand, repoLink);
		auto process 		= ChildProcess{ command, "", ch::seconds{10} };
		auto listExitStatus	= process.runSync();

		if (listExitStatus.value_or(1) != 0)
		{
			throw PaccException(DependencyNotFound, repoLink);
		}

		output = std::move(process.out.stdOut);
	}

	auto versions = PackageVersions::parse(output).sort();

	{
		VersionReq req;
		if (args.size() >= 4 && args[3][0] != '-')
		{
			req = VersionReq::fromString( std::string(args[3]) );
			versions = versions.filter(req);
		}
		else
		{
			fmt::print(boldBlue, "Note: you filter compatible versions, f.e.: \"pacc lsver fmt ^7.1\"\n\n" );
		}

		bool showTags = this->containsSwitch("--tags");

		fmt::print("PACKAGE {}:\n", showTags ? "TAGS" : "VERSIONS");
		if (showTags)
		{
			fmt::print(boldBlue, "Note: viewing real git tags. Remove \"--tags\" param to see version syntax.\n" );
		}

		// Print compatible:
		{
			int NumPerRow = showTags ? 3 : 4;
			size_t numVer = versions.confirmed.size();

			fmt::print("Compatible {}:\n", showTags ? "tags (prefix \"pacc-\" is required)" : "versions");
			if (numVer > 0)
			{
				for(size_t i = 0; i < numVer; i += NumPerRow)
				{
					for (int j = 0; (j < NumPerRow && i + j < numVer); ++j)
					{
						auto const& v = versions.confirmed[i + j];
						if (showTags)
							fmt::print("{:>16}", v.first);
						else
							fmt::print("{:>12}", v.second.toString());
					}
					fmt::print("\n");
				}
			}
			else
				fmt::print( boldBlue, "    None\n" );
		}

		// Print rest:
		if (this->containsSwitch("--all"))
		{
			constexpr int NumPerRow = 4;
			size_t numVer = versions.rest.size();

			fmt::print("Unconfirmed {}:\n", showTags ? "tags" : "versions (prefix \"!\" is required)");
			if (numVer > 0)
			{
				for(size_t i = 0; i < numVer; i += 4)
				{
					for (int j = 0; (j < 4 && i + j < numVer); ++j)
					{
						auto const& v = versions.rest[i + j];
						if (showTags)
							fmt::print("{:>12}", v.first);
						else
							fmt::print("{:>12}", "!" + v.second.toString());
					}
					fmt::print("\n");
				}
			}
			else
				fmt::print( boldBlue, "    None\n" );

		}
	}

}
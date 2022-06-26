#include PACC_PCH

#include <Pacc/App/App.hpp>

#include <Pacc/System/Environment.hpp>


///////////////////////////////////////////////////
void PaccApp::listPackages()
{
	using fmt::fg, fmt::color;

	auto boldBlue 	= fmt::emphasis::bold | fg(color::light_sky_blue);
	auto boldYellow = fmt::emphasis::bold | fg(color::yellow);

	auto packagesRoot = env::getPaccDataStorageFolder() / "packages";

	std::string filter = "";

	if (args.size() >= 3)
		filter = args[2];

	size_t totalCount = 0;
	size_t linksCount = 0;

	struct PackageInfo {
		std::string name;
		std::string linkTo;
		Version		ver;
	};

	auto pkgs = std::vector<PackageInfo>();

	if (fs::is_directory(packagesRoot))
	{
		pkgs.reserve(100);

		// Collect info
		for (auto entry : fs::directory_iterator(packagesRoot))
		{
			if (!filter.empty() && entry.path().filename().string().find(filter) == std::string::npos)
				continue;

			bool dir = fs::is_directory(entry);
			bool sym = fs::is_symlink(entry);
			if (dir || sym)
			{
				auto packageFile = findPackageFile(entry.path());
				if (packageFile.empty())
					continue;

				std::unique_ptr<Package> pkg;
				try {
					pkg = this->loadPackage(entry.path());
				}
				catch (PaccException& e) {
					fmt::print("{}\n", e.what());
					continue;
				}
				catch (std::exception& e) {
					fmt::print("{}\n", e.what());
				}
				catch (...) {
					continue;
				}


				++totalCount;
				auto pkgInfo = PackageInfo();
				pkgInfo.ver = pkg->version;

				pkgInfo.name = entry.path().filename().string();

				if (sym)
				{
					++linksCount;
					pkgInfo.linkTo = fs::read_symlink(entry).string();
				}

				pkgs.emplace_back(std::move(pkgInfo));
			}
		}
	}

	// Print stats
	{
		fmt::print(boldBlue, "Stats");
		if (filter.empty())
			fmt::print(": \n");
		else
			fmt::print(" (filtered by: \"{}\"): \n", filter);

		fmt::print("{} ", totalCount);
		fmt::print(fg(color::light_sky_blue), "packages installed globally (");
		fmt::print("{} ", linksCount);
		fmt::print(fg(color::light_sky_blue), "symlinks to a local version)\n");
		fmt::print(boldBlue, "Packages");
		fmt::print(": \n");
	}

	// Print packages
	{
		auto linkDirectoryStyle	= fg(color::dim_gray);
		auto linkArrowStyle		= fg(color::aqua);
		auto linkDashStyle		= fg(color::aqua);
		auto versionStyle		= fg(color::dark_slate_gray);

		for (auto& pkg : pkgs)
		{
			auto isLink = !pkg.linkTo.empty();


			if (isLink)
				fmt::print(linkDashStyle, " - ");
			else
				fmt::print(" - ");

			fmt::print("{:<55}", fmt::format("{} ", pkg.name) + fmt::format(versionStyle, "@{}", pkg.ver.toString()));
			if (!isLink)
				fmt::print("\n");
			else
			{
				fmt::print(linkArrowStyle, " -> ");
				fmt::print(linkDirectoryStyle, "{}\n", pkg.linkTo);
			}
		}
	}

}

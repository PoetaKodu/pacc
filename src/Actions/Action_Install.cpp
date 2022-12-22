#include "include/Pacc/PaccPCH.hpp"

#include <Pacc/App/App.hpp>

#include <Pacc/Helpers/Exceptions.hpp>
#include <Pacc/System/Environment.hpp>
#include <Pacc/System/Filesystem.hpp>
#include <Pacc/App/Help.hpp>


///////////////////////////////////////////////////
auto PaccApp::install() -> void
{
	using fmt::fg, fmt::color;

	bool global = settings.isFlagSet("-g");

	auto targetPath = fs::path();
	if (global)
		targetPath = env::requirePaccDataStorageFolder() / "packages";
	else
		targetPath = "pacc_packages";

	// Iterate over every non-flag argument provieded **after** the main action name
	size_t numRequested = 0;

	// This loop is just for the `numRequested` variable (for now)
	for (size_t i = settings.actionNameIndex + 1; i < args.size(); ++i)
	{
		if (settings.wasParsed(i))
			continue;

		++numRequested;
	}

	size_t numDownloaded = 0;

	for (size_t i = settings.actionNameIndex + 1; i < args.size(); ++i)
	{
		if (settings.wasParsed(i))
			continue;

		auto packageTemplate = String(args[i]);

		auto loc = DownloadLocation::parse( packageTemplate );

		auto targetPackagePath = targetPath / loc.repository;
		if (fs::is_directory(targetPackagePath) || fs::is_symlink(targetPackagePath)) // works for both symlinks and junctions (is_directory is true)
		{
			throw PaccException("Package \"{0}\" is already installed{1}.", loc.repository, global ? " globally" : "")
				.withHelp("Uninstall the package with \"pacc uninstall {0}{1}\"", loc.repository, global ? " --global" : "");
		}

		this->downloadPackage(targetPackagePath, loc);
		++numDownloaded;

		fmt::print(fg(color::lime_green), "Installed package \"{}\" ({} of {}).\n", loc.repository, numDownloaded, numRequested);
	}

	if (numRequested == 0)
	{
		if (global)
		{
			throw PaccException("Missing argument: package name")
				.withHelp("Use \"pacc install [package_name] [package_name_2] [...] --global\"");
		}

		auto pkg = Package::load();

		size_t numInstalled = 0;
		try {
			numInstalled = this->installPackageDependencies(*pkg, true);
		}
		catch(...)
		{
			fmt::printErr(fg(color::red), "Installed {} packages.\n", numInstalled);
			throw;
		}
		if (numInstalled > 0)
			fmt::print(fg(color::lime_green), "Installed {} packages.\n", numInstalled);
	}
}

///////////////////////////////////////////////////
auto PaccApp::uninstall() -> void
{
	using fmt::fg, fmt::color;

	bool global = settings.flags.at("-g")->isSet();

	auto targetPath = fs::path();
	if (global)
		targetPath = env::requirePaccDataStorageFolder() / "packages";
	else
		targetPath = "pacc_packages";

	// Iterate over every non-flag argument provieded **after** the main action name
	size_t numRequested = 0;
	for (size_t i = settings.actionNameIndex + 1; i < args.size(); ++i)
	{
		if (settings.wasParsed(i))
			continue;

		++numRequested;

		auto packageName = String(args[i]);
		auto packagePath = targetPath / packageName;

		if (fsx::isSymlinkOrJunction(packagePath) && global)
		{
			fsx::makeWritableAll(packagePath);
			fs::remove(packagePath);
			fmt::print("Package \"{}\" has been unlinked from the user environment.", packageName);
		}
		else if (fs::is_directory(packagePath))
		{
			fsx::makeWritableAll(packagePath);
			fs::remove_all(packagePath);
			fmt::print(fg(color::lime_green), "Uninstalled package \"{}\".\n", packageName);
		}
		else
		{
			if (fs::exists(packagePath))
			{
				throw PaccException("Invalid type of package \"{}\".", packageName)
					.withHelp("Directory or symlink required");
			}
			else
			{
				throw PaccException("Package \"{0}\" is not installed{1}.", packageName, global ? " globally" : "");
			}
		}
	}

	if (numRequested == 0)
	{
		throw PaccException("Missing argument: package name")
			.withHelp("Use \"pacc uninstall [package_name] [package_name_2] [...]\"");
	}
}


///////////////////////////////////////////////////
auto PaccApp::installPackageDependencies(Package& pkg, bool isRoot) -> size_t
{
	using fmt::fg, fmt::color;

	auto targetPath = fs::path(isRoot ? "pacc_packages" : "..");

	auto deps = this->collectMissingDependencies(pkg);

	if (deps.empty())
	{
		if (isRoot) {
			fmt::print("No packages to install.\n");
		}

		return 0;
	}

	size_t numInstalled = 0;

	auto installed = Vec<fs::path>();

	for (auto const& dep : deps)
	{
		auto loc = DownloadLocation::parse( dep.downloadLocation );
		if (loc.platform == DownloadLocation::Unknown)
		{
			throw PaccException("Missing package \"{}\" with no download location specified, or the location is wrong.", dep.packageName)
				.withHelp(
						"Provide \"from\" for the package. Use following syntax:\n{}",
						help::DependencySyntax
					);
		}

		auto targetPackagePath = targetPath / dep.packageName;
		if (fs::is_directory(targetPackagePath) || fs::is_symlink(targetPackagePath)) // works for both symlinks and junctions (is_directory is true)
		{
			throw PaccException("Package folder \"{}\" is already used.", targetPackagePath.filename().string())
				.withHelp("Remove the folder.");
		}


		this->downloadPackage(targetPackagePath, loc);

		installed.emplace_back(std::move(targetPackagePath));

		// TODO: run install script on package.


		++numInstalled;
	}

	auto prevPath = fs::current_path();
	for (auto pkgPath : installed)
	{
		fs::current_path(pkgPath);
		try {
			auto pkg = Package::load();
			numInstalled += this->installPackageDependencies(*pkg, false);
		}
		catch(...) {
			fs::current_path(prevPath); // Make sure that we are at valid path
			throw; // ... and rethrow
		}
		fs::current_path(prevPath);
	}

	return numInstalled;
}

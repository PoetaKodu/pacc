#include PACC_PCH

#include <Pacc/App/App.hpp>

#include <Pacc/Helpers/Exceptions.hpp>
#include <Pacc/System/Environment.hpp>
#include <Pacc/System/Filesystem.hpp>
#include <Pacc/App/Help.hpp>


///////////////////////////////////////////////////
void PaccApp::install()
{
	using fmt::fg, fmt::color;

	bool global = false;
	if (this->containsSwitch("-g") || this->containsSwitch("--global"))
		global = true;

	fs::path targetPath;
	if (global)
		targetPath = env::requirePaccDataStorageFolder() / "packages";
	else
		targetPath = "pacc_packages";

	 // TODO: improve this
	if (args.size() >= (global ? 4 : 3))
	{
		std::string packageTemplate(args[2]);

		auto loc = DownloadLocation::parse( packageTemplate );

		fs::path targetPackagePath = targetPath / loc.repository;
		if (fs::is_directory(targetPackagePath) || fs::is_symlink(targetPackagePath))
		{
			throw PaccException("Package \"{0}\" is already installed{1}.", loc.repository, global ? " globally" : "")
				.withHelp("Uninstall the package with \"pacc uninstall {0}{1}\"", loc.repository, global ? " --global" : "");
		}

		this->downloadPackage(targetPackagePath, loc);

		fmt::print(fg(color::lime_green), "Installed package \"{}\".\n", loc.repository);
	}
	else
	{
		if (global)
			throw PaccException("Missing argument: package name")
				.withHelp("Use \"pacc install [package_name] --global\"");

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
void PaccApp::uninstall()
{
	using fmt::fg, fmt::color;

	bool global = false;
	if (this->containsSwitch("-g") || this->containsSwitch("--global"))
		global = true;

	fs::path targetPath;
	if (global)
		targetPath = env::requirePaccDataStorageFolder() / "packages";
	else
		targetPath = "pacc_packages";

	 // TODO: improve this
	if (args.size() >= (global ? 4 : 3))
	{
		std::string packageName(args[2]);

		fs::path packagePath = targetPath / packageName;

		if (fs::is_symlink(packagePath) && global)
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
	else
		throw PaccException("Missing argument: package name")
			.withHelp("Use \"pacc uninstall [package_name]\"");
}


///////////////////////////////////////////////////
size_t PaccApp::installPackageDependencies(Package& pkg_, bool isRoot)
{
	using fmt::fg, fmt::color;

	fs::path targetPath = isRoot ? "pacc_packages" : "../pacc_packages";

	auto deps = this->collectMissingDependencies(pkg_);
	
	if (deps.empty())
	{
		if (isRoot)
			fmt::print("No packages to install.\n");
		return 0;
	}

	size_t numInstalled = 0;

	std::vector<fs::path> installed;

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

		fs::path targetPackagePath = targetPath / dep.packageName;
		if (fs::is_directory(targetPackagePath) || fs::is_symlink(targetPackagePath))
		{
			throw PaccException("Package folder \"{}\" is already used.", targetPackagePath.filename().string())
				.withHelp("Remove the folder.");
		}


		this->downloadPackage(targetPackagePath, loc);

		installed.emplace_back(std::move(targetPackagePath));
		
		// TODO: run install script on package.


		++numInstalled;
	}

	fs::path prevPath = fs::current_path();
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

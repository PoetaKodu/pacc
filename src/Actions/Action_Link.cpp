#include PACC_PCH

#include <Pacc/App/App.hpp>

#include <Pacc/System/Filesystem.hpp>
#include <Pacc/Helpers/Exceptions.hpp>
#include <Pacc/System/Environment.hpp>


///////////////////////////////////////////////////
void PaccApp::linkPackage()
{
	auto pkg = Package::load();

	fs::path appData = env::getPaccDataStorageFolder();

	fs::path packagesDir 	= appData / "packages";
	fs::path targetSymlink 	= packagesDir / pkg->name;

	fs::create_directories(packagesDir);

	if (fs::exists(targetSymlink))
	{
		if (fsx::isSymlinkOrJunction(targetSymlink))
		{
			throw PaccException(
					"Package \"{}\" is already linked to {}.\n",
					pkg->name,
					fsx::readSymlinkOrJunction(targetSymlink).string()
				)
				.withHelp("If you want to update the link, use \"pacc unlink\" first.");
		}
		else
		{
			throw PaccException(
					"Package \"{}\" is already installed in users environment.\n",
					pkg->name
				)
				.withHelp("If you want to link current package, uninstall existing one with \"pacc uninstall\" first.");
		}
	}
	else
	{
		fsx::createSymlink(fs::current_path(), targetSymlink, true);
		fmt::print("Package \"{}\" has been linked inside the user environment.", pkg->name);
	}
}

///////////////////////////////////////////////////
void PaccApp::unlinkPackage()
{
	String pkgName;
	if (args.size() > 2)
		pkgName = args[2];

	if (pkgName.empty())
	{
		auto pkg = Package::load();
		pkgName = pkg->name;
	}

	fs::path storage = env::getPaccDataStorageFolder();
	fs::path symlinkPath = storage / "packages" / pkgName;
	if (fsx::isSymlinkOrJunction(symlinkPath))
	{
		fs::remove(symlinkPath);
		fmt::print("Package \"{}\" has been unlinked from the user environment.", pkgName);
	}
	else
	{
		throw PaccException(
				"Package \"{}\" is not linked within user environment.\n",
				pkgName
			).withHelp("If you want to link current package, use \"pacc link\" first.");
	}
}

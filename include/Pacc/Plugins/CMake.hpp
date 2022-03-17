#pragma once

#include PACC_PCH

#include <Pacc/PackageSystem/IPackageLoader.hpp>

namespace plugins::cmake
{

struct BuildInfo {
	std::map<std::string, fs::path> artifacts;

	// TODO: add more info
};

BuildInfo runBuildInfoQuery(fs::path const& packagePath_);

class PackageLoader
	: public IPackageLoader
{
public:
	using IPackageLoader::IPackageLoader;


	bool canLoad(fs::path const& root_) const override
	{
		return fs::is_directory(root_) && fs::is_regular_file(root_ / "CMakeLists.txt");
	}

	UPtr<Package> load(fs::path const& root_) override;

	bool loadTarget(fs::path const& root_, std::string const& name_, TargetBase& target_) override;
};

}

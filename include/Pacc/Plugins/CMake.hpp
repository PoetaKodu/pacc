#pragma once

#include PACC_PCH

#include <Pacc/PackageSystem/IPackageLoader.hpp>
#include <Pacc/Build/IPackageBuilder.hpp>

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

	auto canLoad(fs::path const& root_) const -> bool override
	{
		return fs::is_directory(root_) && fs::is_regular_file(root_ / "CMakeLists.txt");
	}

	auto load(fs::path const& root_) -> UPtr<Package> override;
	auto loadTarget(fs::path const& root_, std::string const& name_, TargetBase& target_) -> bool override;

protected:
	auto discoverTargets(json const& json_) const -> Vec<StringPair>;
	auto loadProjectFromFile(fs::path const& file, Project& project_) -> bool;
	auto loadVersion(fs::path const& root_) -> Version;
};


class PackageBuilder
	: public IPackageBuilder
{
public:
	using IPackageBuilder::IPackageBuilder;

	auto run(Package const& pkg_, Toolchain& tc_, BuildSettings const& settings_ = {}, int verbosityLevel_ = 0) -> BuildProcessResult override;
};

}

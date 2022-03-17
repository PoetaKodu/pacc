#pragma once

#include PACC_PCH

#include <Pacc/PackageSystem/IPackageLoader.hpp>

class MainPackageLoader
	: public IPackageLoader
{
public:
	using IPackageLoader::IPackageLoader;

	UPtr<Package> load(fs::path const& root_) override;
	bool canLoad(fs::path const& root_) const override;
	bool loadTarget(fs::path const& root_, std::string const& name_, TargetBase& target_) override;
};

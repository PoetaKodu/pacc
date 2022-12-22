#pragma once

#include <Pacc/PaccPCH.hpp>

#include <Pacc/PackageSystem/IPackageLoader.hpp>

class MainPackageLoader
	:
	public IPackageLoader
{
public:
	using IPackageLoader::IPackageLoader;

	auto load(fs::path const& root_) -> UPtr<Package> override;
	auto canLoad(fs::path const& root_) const -> bool override;
	auto loadTarget(fs::path const& root_, String const& name_, TargetBase& target_) -> bool override;
};

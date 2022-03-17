#pragma once

#include PACC_PCH

#include <Pacc/PackageSystem/Package.hpp>

class PaccApp;

class IPackageLoader
{
public:
	PaccApp& app;

	explicit IPackageLoader(PaccApp& app_)
		: app(app_)
	{}

	virtual ~IPackageLoader() {}
	virtual UPtr<Package> load(fs::path const& root_) = 0;

	// Utilized during loader autodetection
	virtual bool canLoad(fs::path const& root_) const = 0;

	virtual bool loadTarget(fs::path const& root_, std::string const& name_, TargetBase& target_) = 0;

	// Lower value means higher priority.
	int autodetectPriority = 0;
};

constexpr inline auto PackageLoaderAutodetectPriorityComp =
		[](IPackageLoader const* lhs, IPackageLoader const* rhs)
		{
			return lhs->autodetectPriority < rhs->autodetectPriority;
		};

#pragma once

#include PACC_PCH

class PaccApp;
struct Package;
struct Toolchain;

/// <summary>
/// A settings container for a build process.
/// </summary>
struct BuildSettings
{
	std::string configName 		= "Debug";
	std::string platformName 	= "x64";

	std::string targetName 		= "";

	std::optional<int> cores;
};

using BuildProcessResult = std::optional<int>;

/// <summary>
/// 	Interface for package builders.
/// </summary>
class IPackageBuilder
{
public:
	PaccApp* app;

	IPackageBuilder(PaccApp& app_)
		: app(&app_)
	{}

	virtual ~IPackageBuilder() {}

	virtual BuildProcessResult run(Package const& pkg_, Toolchain& tc_, BuildSettings const& settings_ = {}, int verbosityLevel_ = 0) = 0;
};

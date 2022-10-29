#pragma once

#include PACC_PCH

#include <Pacc/Helpers/HelperTypes.hpp>

class PaccApp;
struct Package;
struct Toolchain;

/// <summary>
/// A settings container for a build process.
/// </summary>
struct BuildSettings
{
	String configName 		= "Debug";
	String platformName 	= "x64";

	String targetName 		= "";

	Opt<int> cores;
};

using BuildProcessResult = Opt<int>;

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

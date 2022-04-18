#pragma once

#include <Pacc/Build/IPackageBuilder.hpp>

class PaccPackageBuilder
	: public IPackageBuilder
{
public:
	using IPackageBuilder::IPackageBuilder;

	BuildProcessResult run(Package const& pkg_, Toolchain& tc_, BuildSettings const& settings_ = {}, int verbosityLevel_ = 0) override;
};

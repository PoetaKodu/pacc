#pragma once

#include <Pacc/Build/IPackageBuilder.hpp>

class PaccPackageBuilder
	: public IPackageBuilder
{
public:
	using IPackageBuilder::IPackageBuilder;

	auto run(
		Package const&			package,
		Toolchain&				toolchain,
		BuildSettings const&	settings = {},
		int						verbosityLevel = 0
	) -> BuildProcessResult override;
};

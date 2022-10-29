#pragma once

#include <Pacc/Toolchains/Toolchain.hpp>

struct GNUMakeToolchain : Toolchain
{
	String cppCompilerName	= "g++";
	String cCompilerName 	= "gcc";

	virtual Type type() const { return GNUMake; }

	virtual bool isEqual(Toolchain const& other_) const override;

	virtual void serialize(json& out_) const override;

	virtual bool deserialize(json const& in_) override;

	// TODO: Choose between "gmake" and "gmake2"
	virtual String premakeToolchainType() const { return "gmake2"; }

	virtual Opt<int> run(Package const & pkg_, BuildSettings settings_ = {}, int verbosityLevel_ = 0) override;

	static Vec<GNUMakeToolchain> detect();
};

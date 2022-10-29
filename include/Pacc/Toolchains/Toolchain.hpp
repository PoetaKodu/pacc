#pragma once

#include PACC_PCH

#include <Pacc/Helpers/Json.hpp>
#include <Pacc/Helpers/Exceptions.hpp>
#include <Pacc/Helpers/HelperTypes.hpp>
#include <Pacc/Build/IPackageBuilder.hpp>

struct Toolchain
{
	String 	prettyName;
	String 	version;

	fs::path 		mainPath;

	enum Type
	{
		MSVC,
		GNUMake,
		Unknown
	};

	virtual ~Toolchain() = default;

	virtual Type type() const;

	virtual Opt<int> run(struct Package const & pkg_, BuildSettings settings_ = {}, int verbosityLevel_ = 0);

	static StringView typeName(Type type_);

	virtual String premakeToolchainType() const;


	virtual bool generateProjectFiles();


	virtual bool isEqual(Toolchain const& other_) const;

	virtual void serialize(json& out_) const;

	virtual bool deserialize(json const& in_);

};

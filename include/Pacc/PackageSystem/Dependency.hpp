#pragma once

#include PACC_PCH

#include <Pacc/Helpers/HelperTypes.hpp>
#include <Pacc/PackageSystem/Version.hpp>

using RawDependency 	= std::string;
using PackagePtr 		= std::shared_ptr<struct Package>;

enum class AccessType
{
	Private,
	Public,
	Interface
};

template <typename T>
struct AccessSplit
{
	T public_;
	T private_;
	T interface_;
};
template <typename T>
using AccessSplitVec 	= AccessSplit<std::vector<T>>;
using VecOfStrAcc 		= AccessSplitVec<std::string>;

template <typename T>
T& targetByAccessType(AccessSplit<T> & accessSplit_, AccessType type_);

struct DownloadLocation
{
	enum Platform
	{
		Unknown,
		GitHub,
		GitLab,
		OfficialRepo // userName is ignored when this is used.
	};

	static DownloadLocation parse(std::string const& depTemplate_);

	std::string getGitLink() const;
	std::string getBranch() const;

	std::string repository;

	std::string userName 	= "";
	std::string branch		= ""; // Branch or a tag.
	Platform platform 		= Unknown;
	bool exactBranch 		= false;
};

using StringVersionPair = std::pair<std::string, Version>;
struct PackageVersions
{
	std::vector<StringVersionPair> confirmed, rest;

	PackageVersions& sort();
	PackageVersions filter(VersionReq const& req_);

	static PackageVersions parse(std::string const& lsRemoteOutput_);
};
	

struct PackageDependency
{
	VecOfStr 		projects;
	std::string 	packageName;
	std::string 	downloadLocation;
	VersionReq 		version{};

	// Resolved (or not) package pointer.
	PackagePtr 		package{};

	// TODO: add config support

};

struct SelfDependency
{
	struct Project*	project;
	std::string 	depProjName;
	struct Package*	package;
};

class Dependency
{
public:	
	AccessType accessType;

	Dependency() = default;

	enum Type
	{
		Raw,
		Package,
		Self,
		None
	};

	bool isRaw() const 		{ return val.index() == 0; }
	bool isPackage() const 	{ return val.index() == 1; }
	bool isSelf() const 	{ return val.index() == 2; }

	Type type() const
	{
		if (this->isRaw())
			return Type::Raw;
		else if (this->isPackage())
			return Type::Package;
		else if (this->isSelf())
			return Type::Self;
		
		return Type::None;
	}	

	// const getters

	auto const& raw() const			{ return std::get<RawDependency>(val); }
	auto const& package() const 	{ return std::get<PackageDependency>(val); }
	auto const& self() const 		{ return std::get<SelfDependency>(val); }

	// getters

	auto& 		raw() 				{ return std::get<RawDependency>(val); }
	auto& 		package() 			{ return std::get<PackageDependency>(val); }
	auto& 		self() 				{ return std::get<Self>(val); }

	static Dependency raw(RawDependency d) 			{ return Dependency{ std::move(d) }; }
	static Dependency package(PackageDependency d) 	{ return Dependency{ std::move(d) }; }
	static Dependency self(SelfDependency d) 		{ return Dependency{ std::move(d) }; }

private:
	using ValType = std::variant<
			RawDependency,
			PackageDependency,
			SelfDependency
		>;
	// Construct from variant
	Dependency(ValType v)
	{
		val = std::move(v);
	}


	ValType val; // variant stored underhood
};



// TODO: add C++20 support to use concepts
template <typename T>
auto getAccesses(std::vector<T> & v)
{
	return std::vector{ &v };
}

/////////////////////////////////////////////////
template <typename T>
auto getAccesses(AccessSplitVec<T> & v)
{
	return std::vector{ &v.private_, &v.public_, &v.interface_ };
}

/////////////////////////////////////////////////
template <typename T>
auto getAccesses(std::vector<T> const & v)
{
	return std::vector{ &v };
}

/////////////////////////////////////////////////
template <typename T>
auto getAccesses(AccessSplitVec<T> const & v)
{
	return std::vector{ &v.private_, &v.public_, &v.interface_ };
}


/////////////////////////////////////////////////
template <typename T>
T& targetByAccessType(AccessSplit<T> & accessSplit_, AccessType type_)
{
	switch(type_)
	{
	case AccessType::Private: 		return accessSplit_.private_;
	case AccessType::Public: 		return accessSplit_.public_;
	case AccessType::Interface: 	return accessSplit_.interface_;
	}

	assert(false); // Should never happen.
	return accessSplit_.private_;
}

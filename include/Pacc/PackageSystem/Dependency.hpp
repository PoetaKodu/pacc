#pragma once

#include <Pacc/PaccPCH.hpp>

#include <Pacc/Helpers/HelperTypes.hpp>
#include <Pacc/PackageSystem/Version.hpp>

using RawDependency 	= String;
using PackagePtr 		= std::shared_ptr<struct Package>;

enum class AccessType
{
	Private,
	Public,
	Interface
};

enum MultiAccess
{
	Private 	= 1 << 0,
	Public		= 1 << 1,
	Interface 	= 1 << 2,

	All 		= Private | Public | Interface,
	NoInterface	= Private | Public,
};

template <typename T>
struct AccessSplit
{
	T public_;
	T private_;
	T interface_;
};
template <typename T>
using AccessSplitVec 	= AccessSplit<Vec<T>>;
using VecOfStrAcc 		= AccessSplitVec<String>;

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

	static DownloadLocation parse(String const& depTemplate_);

	String getGitLink() const;
	String getBranch() const;

	String repository;

	String userName 	= "";
	String branch		= ""; // Branch or a tag.
	Platform platform 		= Unknown;
	bool exactBranch 		= false;
};

using StringVersionPair = std::pair<String, Version>;
struct PackageVersions
{
	Vec<StringVersionPair> confirmed, rest;

	PackageVersions& sort();
	PackageVersions filter(VersionReq const& req_);

	static PackageVersions parse(String const& lsRemoteOutput_);
};


struct PackageDependency
{
	Vec<String> 		projects;
	String 	packageName;
	String 	downloadLocation;
	VersionReq 		version{};

	// Resolved (or not) package pointer.
	PackagePtr 		package{};

	// TODO: add config support

};

struct SelfDependency
{
	struct Project*	project;
	String 	depProjName;
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
auto getAccesses(Vec<T> & v, MultiAccess mode_ = MultiAccess::All)
{
	return Vec< Vec<T>* >{ &v };
}

/////////////////////////////////////////////////
template <typename T>
auto getAccesses(AccessSplitVec<T> & v, MultiAccess mode_ = MultiAccess::All)
{
	using ValType = Vec<T> *;

	auto result = Vec<ValType>{};
	result.reserve(3);

	if (mode_ & MultiAccess::Private)
		result.push_back(&v.private_);
	if (mode_ & MultiAccess::Public)
		result.push_back(&v.public_);
	if (mode_ & MultiAccess::Interface)
		result.push_back(&v.interface_);

	return result;
}

/////////////////////////////////////////////////
template <typename T>
auto getAccesses(Vec<T> const & v, MultiAccess mode_ = MultiAccess::All)
{
	return Vec<Vec<T> const*>{ &v };
}

/////////////////////////////////////////////////
template <typename T>
auto getAccesses(AccessSplitVec<T> const & v, MultiAccess mode_ = MultiAccess::All)
{
	using ValType = Vec<T> const*;

	auto result = Vec<ValType>{};
	result.reserve(3);

	if (mode_ & MultiAccess::Private)
		result.push_back(&v.private_);
	if (mode_ & MultiAccess::Public)
		result.push_back(&v.public_);
	if (mode_ & MultiAccess::Interface)
		result.push_back(&v.interface_);

	return result;
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

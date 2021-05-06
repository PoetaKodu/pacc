#pragma once

#include PACC_PCH

#include <Pacc/HelperTypes.hpp>

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

struct PackageDependency
{
	VecOfStr 	projects;
	std::string packageName;
	std::string version{};

	// Resolved (or not) package pointer.
	PackagePtr 	package{};

	// TODO: add config support

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
		None
	};

	bool isRaw() const 		{ return val.index() == 0; }
	bool isPackage() const 	{ return val.index() == 1; }

	Type type() const
	{
		if (this->isRaw())
			return Type::Raw;
		else if (this->isPackage())
			return Type::Package;
		
		return Type::None;
	}	

	// const getters

	auto const& raw() const			{ return std::get<RawDependency>(val); }
	auto const& package() const 	{ return std::get<PackageDependency>(val); }

	// getters

	auto& 		raw() 				{ return std::get<RawDependency>(val); }
	auto& 		package() 			{ return std::get<PackageDependency>(val); }

	static Dependency raw(RawDependency d) 			{ return Dependency{ std::move(d) }; }
	static Dependency package(PackageDependency d) 	{ return Dependency{ std::move(d) }; }

private:
	using ValType = std::variant<RawDependency, PackageDependency>;
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



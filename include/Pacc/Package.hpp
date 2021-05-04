#pragma once

#include PACC_PCH

constexpr std::string_view PackageJSON 	= "cpackage.json";
constexpr std::string_view PackageLUA 	= "cpackage.lua";

using VecOfStr 		= std::vector< std::string >;
using VecOfStrPtr 	= std::vector< std::string* >;
using PackagePtr 	= std::shared_ptr<struct Package>;

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
using RawDependency 	= std::string;

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
	
	using ValType = std::variant<RawDependency, PackageDependency>;
	ValType val;
	Dependency(ValType v) {
		val = std::move(v);
	}
public:	
	Dependency() = default;

	enum Type
	{
		Raw,
		Package,
		None
	};

	bool isRaw() const {
		return val.index() == 0;
	}
	bool isPackage() const {
		return val.index() == 1;
	}

	Type type() const
	{
		if (this->isRaw())
			return Type::Raw;
		else if (this->isPackage())
			return Type::Package;
		
		return Type::None;
	}	

	auto const& raw() const {
		return std::get<RawDependency>(val);
	}
	auto const& package() const {
		return std::get<PackageDependency>(val);
	}
	
	auto& raw() {
		return std::get<RawDependency>(val);
	}
	auto& package() {
		return std::get<PackageDependency>(val);
	}

	static Dependency raw(RawDependency d) {
		return Dependency{ std::move(d) }; 
	}
	static Dependency package(PackageDependency d) {
		return Dependency{ std::move(d) }; 
	}
};




struct TargetBase
{
	std::string 	name;

	template <typename T>
	struct SelfAndComputed {
		T self;
		T computed;
	};
	template <typename T>
	using SaC = SelfAndComputed<T>;

	VecOfStr					 	files;
	SaC<AccessSplitVec<Dependency>> dependencies;
	SaC<VecOfStrAcc>			 	defines;
	SaC<VecOfStrAcc>			 	includeFolders;
	SaC<VecOfStrAcc>			 	linkerFolders;
	SaC<VecOfStrAcc>			 	linkedLibraries;
};

struct Project : TargetBase
{
	std::string		type;
	std::string		language;
};

struct Package : TargetBase
{
	fs::path 		root;
	std::vector<Project> projects;

	static Package load(fs::path dir_ = "");

	Project const* findProject(std::string_view name_) const;

private:
	static Package loadFromJSON(std::string const& packageContent_);
};





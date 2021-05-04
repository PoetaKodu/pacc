#pragma once

#include PACC_PCH

constexpr std::string_view PackageJSON 	= "cpackage.json";
constexpr std::string_view PackageLUA 	= "cpackage.lua";

using VecOfStr 		= std::vector< std::string >;
using PackagePtr 	= std::shared_ptr<struct Package>;
struct VecOfStrAcc
{
	VecOfStr public_;
	VecOfStr private_;
	VecOfStr interface_;
};


using RawDependency = std::string;

struct PackageDependency
{
	std::string projectName;
	std::string packageName;
	std::string version;

	// Resolved (or not) package pointer.
	PackagePtr 	package;

	// TODO: add config support

	static PackageDependency from(std::string_view depPattern);
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

	VecOfStr 				files;
	std::vector<Dependency> dependencies;
	VecOfStrAcc 			defines;
	VecOfStrAcc 			includeFolders;
	VecOfStrAcc 			linkerFolders;
	VecOfStrAcc 			linkedLibraries;
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





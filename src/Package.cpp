#include BLOCC_PCH

#include <Blocc/Package.hpp>
#include <Blocc/Errors.hpp>
#include <Blocc/Readers/General.hpp>
#include <Blocc/Readers/JSONReader.hpp>


///////////////////////////////////////////////////////////////
Dependency Dependency::from(std::string_view depPattern)
{
	if (depPattern.empty())
		throw std::runtime_error(fmt::format("Invalid dependency pattern \"{}\"", depPattern));

	auto pkgSep = depPattern.find(':');
	
	if (pkgSep != std::string::npos)
	{
		return Dependency{
				std::string(depPattern.substr(0, pkgSep)),
				std::string(depPattern.substr(pkgSep + 1))
			};	
	}
	return Dependency{ std::string(""), std::string(depPattern) };
}

///////////////////////////////////////////////////
Package Package::load(fs::path dir_)
{
	if (dir_.empty()) {
		dir_ = fs::current_path();
	}

	enum class PackageFileSource
	{
		JSON,
		LuaScript
	};

	PackageFileSource pkgSrcFile;
	
	Package pkg;

	// Detect package file
	if (fs::exists(dir_ / PackageLUA)) // LuaScript has higher priority
	{
		pkgSrcFile = PackageFileSource::LuaScript;
		pkg.root = dir_ / PackageLUA;
	}
	else if (fs::exists(dir_ / PackageJSON))
	{
		pkgSrcFile = PackageFileSource::JSON;
		pkg.root = dir_ / PackageJSON;
	}
	else
		throw std::exception(errors::NoPackageSourceFile.data());
	

	// Decide what to do:
	switch(pkgSrcFile)
	{
	case PackageFileSource::JSON:
	{
		std::cout << "Loading \"" << PackageJSON << "\" file\n";\

		pkg = reader::loadFromJSON(reader::readFileContents(PackageJSON));
		break;
	}
	case PackageFileSource::LuaScript:
	{
		std::cout << "Loading \"" << PackageLUA << "\" file\n";

		// TODO: implement this.
		std::cout << "This function is not implemented yet." << std::endl;
		break;
	}
	}
	return pkg;
}

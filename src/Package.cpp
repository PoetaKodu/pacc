#include PACC_PCH

#include <Pacc/Package.hpp>
#include <Pacc/Errors.hpp>
#include <Pacc/Readers/General.hpp>
#include <Pacc/Readers/JSONReader.hpp>


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

		pkg = Package::loadFromJSON(reader::readFileContents(dir_ / PackageJSON));
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


///////////////////////////////////////////////////
Project const* Package::findProject(std::string_view name_) const
{
	auto it = std::find_if(projects.begin(), projects.end(),
		[&](auto const& e) {
			return e.name == name_;
		});

	if (it != projects.end())
		return &(*it);

	return nullptr;
}

///////////////////////////////////////////////////
Package Package::loadFromJSON(std::string const& packageContent_)
{
	using namespace reader;


	Package result;

	// Parse and make conformant:
	json j;
	PackageJSONView view{ j };

	j = json::parse(packageContent_);
	view.makeConformant();
	
	// std::ofstream("package.dump.json") << j.dump(1, '\t');

	// Load JSON:
	result.name = j["name"].get<std::string>();

	auto projects = j.find("projects");

	result.projects.reserve(projects->size());

	auto loadVecOfStrField = [](json const &j, std::string_view fieldName, bool direct = false, bool required = false)
		{
			VecOfStr result;
			std::string const elemName = std::string(fieldName) + " element";

			// Either subfield or the `j` itself (direct => `j` is an array)
			json const* val = nullptr;
			if (direct)
				val = &j;
			else if (auto it = j.find(fieldName); it != j.end())
				val = &it.value();
			else
			{
				if (required)
					throw std::runtime_error(fmt::format("field {0} not found", fieldName));
				else
					return result;
			}
			if (val->type() == json::value_t::string)
			{
				result.push_back(*val);
			}
			else
			{
				PackageJSONView::expectType(*val, fieldName, json::value_t::array);
				
				// Read the array:
				result.reserve(val->size());

				for(auto elem : val->items())
				{
					PackageJSONView::expectType(elem.value(), elemName, json::value_t::string);
					result.push_back(elem.value());
				}
			}
			return result;
		};

	auto loadVecOfStrAccField = [&](json const &j, std::string_view fieldName)
		{
			VecOfStrAcc result;
			if (auto it = j.find(fieldName); it != j.end())
			{
				if (it.value().type() == json::value_t::array)
					result.private_ = loadVecOfStrField(*it, fieldName, true);
				else
				{
					result.private_ 	= loadVecOfStrField(*it, "private");
					result.public_ 		= loadVecOfStrField(*it, "public");
					result.interface_ 	= loadVecOfStrField(*it, "interface");
				}
			}	
			return result;
		};

	for(auto it : projects->items())
	{
		auto& jsonProject = it.value();

		Project project;

		project.name 			= jsonProject["name"].get<std::string>();
		project.type 			= jsonProject["type"].get<std::string>();

		// TODO: type and value validation
		if (auto it = jsonProject.find("language"); it != jsonProject.end())
			project.language = it->get<std::string>();

		// TODO: type and value validation
		if (auto it = jsonProject.find("cppStandard"); it != jsonProject.end())
			project.cppStandard = it->get<std::string>();

		// TODO: type and value validation
		if (auto it = jsonProject.find("cStandard"); it != jsonProject.end())
			project.cStandard = it->get<std::string>();

		VecOfStr dependencyPatterns;
		dependencyPatterns 		= loadVecOfStrField(jsonProject, "dependencies");
		project.files 			= loadVecOfStrField(jsonProject, "files");
		project.defines 		= loadVecOfStrAccField(jsonProject, "defines");
		project.includeFolders 	= loadVecOfStrAccField(jsonProject, "includeFolders");
		project.linkedLibraries = loadVecOfStrAccField(jsonProject, "linkedLibraries");
		project.linkerFolders 	= loadVecOfStrAccField(jsonProject, "linkerFolders");
		
		project.dependencies.reserve(dependencyPatterns.size());
		for(std::string_view p : dependencyPatterns)
		{
			project.dependencies.push_back( Dependency::from(p) );
		}
		
		result.projects.push_back(std::move(project));
	}

	return result;
}



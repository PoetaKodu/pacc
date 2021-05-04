#include PACC_PCH

#include <Pacc/Package.hpp>
#include <Pacc/Errors.hpp>
#include <Pacc/Readers/General.hpp>
#include <Pacc/Readers/JSONReader.hpp>


using StringPair = std::pair<std::string, std::string>;

StringPair splitBy(std::string_view s, char c)
{
	auto pos = s.find(c);
	if (pos != std::string_view::npos)
		return StringPair{ s.substr(0, pos), s.substr(pos + 1) };
	else
		return StringPair{ s, std::string{} };
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
template <json::value_t type>
json* expect(json &j)
{
	if (j.type() == type)
		return &j;
	else
		return nullptr;
}

///////////////////////////////////////////////////
template <json::value_t type>
json* expectSub(json &j, std::string_view subfieldName)
{
	auto it = j.find(subfieldName);
	if (it != j.end() && it->type() == type)
	{
		return (&(*it));
	}

	return nullptr;
}

///////////////////////////////////////////////////
template <json::value_t type>
json& require(json &j)
{
	if (j.type() == type)
		return j;
	else
		throw std::runtime_error("invalid type");
}

///////////////////////////////////////////////////
template <json::value_t type>
json& requireSub(json &j, std::string_view subfieldName)
{
	auto it = j.find(subfieldName);
	if (it != j.end() && it->type() == type)
	{
		return (*it);
	}

	throw std::runtime_error("invalid subfield type");
}

void readDependencyAccess(json &deps_, std::vector<Dependency> &target_)
{
	using json_vt = json::value_t;

	if (deps_.type() != json_vt::array)
		throw std::runtime_error("invalid type of dependencies subfield - array required");

	// TODO: change this
	target_.reserve(deps_.size());

	for(auto &item : deps_.items())
	{
		if (json* rawDep = expect<json_vt::string>(item.value()))
		{
			target_.push_back(
					Dependency::raw( std::move( rawDep->get<std::string>() ) )
				);
		}
		else if (json* pkgDep = expect<json_vt::object>(item.value()))
		{
			// Required fields:
			json& name 		= requireSub<json_vt::string>(*pkgDep, "name");
			json& projects 	= requireSub<json_vt::array>(*pkgDep, "projects");
			// Optional fields:
			json* version 	= expectSub<json_vt::string>(*pkgDep, "version");

			// Configure dependency:
			PackageDependency pd;

			// Required:
			pd.packageName = name;
			
			pd.projects.reserve(projects.size());
			for(auto & proj : projects.items())
			{
				json& projName = require<json_vt::string>(proj.value());

				pd.projects.push_back(projName.get<std::string>());
			}

			// Optional
			if (version) {
				pd.version = version->get<std::string>();
			}

			target_.push_back(
					Dependency::package( std::move(pd) )
				);
		}
		else
			throw std::runtime_error("Invalid dependency type");
	}

	
}


///////////////////////////////////////////////////
Package Package::loadFromJSON(std::string const& packageContent_)
{
	using namespace reader;

	using json_vt = json::value_t;

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

		project.files		 			= loadVecOfStrField(jsonProject, "files");
		project.defines.self	 		= loadVecOfStrAccField(jsonProject, "defines");
		project.includeFolders.self	 	= loadVecOfStrAccField(jsonProject, "includeFolders");
		project.linkerFolders.self	 	= loadVecOfStrAccField(jsonProject, "linkerFolders");
		
		// TODO: move to other function:
		auto depsIt = jsonProject.find("dependencies");
		if (depsIt != jsonProject.end())
		{
			auto& deps = depsIt.value();
			auto& projSelfDeps = project.dependencies.self;
			if (deps.type() == json_vt::array)
			{
				readDependencyAccess(*depsIt, projSelfDeps.private_);
			}
			else if (deps.type() == json_vt::object)
			{
				if (deps.contains("public")) 		readDependencyAccess(deps["public"], projSelfDeps.public_);
				if (deps.contains("private")) 		readDependencyAccess(deps["private"], projSelfDeps.private_);
				if (deps.contains("interface")) 	readDependencyAccess(deps["interface"], projSelfDeps.interface_);
			}
			else
				throw std::runtime_error("Invalid type of \"dependencies\" field (must be an array or an object)");
		}
		
		result.projects.push_back(std::move(project));
	}

	return result;
}



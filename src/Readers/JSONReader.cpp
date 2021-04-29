#include <CppPkg/Readers/JSONReader.hpp>

#include <fmt/core.h> 
#include <fstream> 

namespace reader
{

//////////////////////////////////////////////////
constexpr std::string_view jsonTypeName(json::value_t type)
{
	switch(type)
	{
		case json::value_t::null:
			return "null";
		case json::value_t::object:
			return "object";
		case json::value_t::array:
			return "array";
		case json::value_t::string:
			return "string";
		case json::value_t::boolean:
			return "boolean";
		case json::value_t::binary:
			return "binary";
		case json::value_t::discarded:
			return "discarded";
		default:
			return "number";
	}
}

///////////////////////////////////////////////////
Package fromJSON(std::string const& packageContent_)
{
	
	Package result;

	// Parse and make conformant:
	json j;
	PackageJSONView view{ j };

	j = json::parse(packageContent_);
	view.makeConformant();
	std::ofstream("package.dump.json") << j.dump(1, '\t');

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

		project.files 			= loadVecOfStrField(jsonProject, "files");

		project.includeFolders 	= loadVecOfStrAccField(jsonProject, "includeFolders");
		project.linkedLibraries = loadVecOfStrAccField(jsonProject, "linkedLibraries");
		project.linkerFolders 	= loadVecOfStrAccField(jsonProject, "linkerFolders");
		
		result.projects.push_back(std::move(project));
	}

	return result;
}


//////////////////////////////////////////////////
void PackageJSONView::expect(json &j, std::string_view name, json::value_t type)
{
	using namespace fmt;

	constexpr std::string_view WrongTypeMsg =
		"field \"{}\" expected to be of type \"{}\", but \"{}\" given instead";
	constexpr std::string_view NoFieldMsg =
		"field \"{}\" expected to be of type \"{}\" does not exist";

	auto it = j.find(name);

	if (it != j.end())
	{
		if (it->type() != type)
			throw std::runtime_error(fmt::format(WrongTypeMsg, name, jsonTypeName(type), it->type_name()) );
	}
	else
		throw std::runtime_error(fmt::format(NoFieldMsg, name, jsonTypeName(type)) );
}

//////////////////////////////////////////////////
void PackageJSONView::expectType(json const& j, std::string_view name, json::value_t type)
{
	using namespace fmt;

	constexpr std::string_view WrongTypeMsg =
		"field \"{}\" expected to be of type \"{}\", but \"{}\" given instead";

	if (j.type() != type)
		throw std::runtime_error(fmt::format(WrongTypeMsg, name, jsonTypeName(type), j.type_name()) );
}

//////////////////////////////////////////////////
void PackageJSONView::makeConformant()
{
	using jtype = json::value_t;

	if (root.is_array())
	{
		json projects = std::move(root);

		root = json::object();
		root["projects"] = std::move(projects);
	}

	if (root.is_object())
	{
		if (auto it = root.find("projects");
			it != root.end())
		{
			// Has to be an array
			expectType(*it, "projects", jtype::array);

			// At least one project
			if (it->size() < 1)
				throw std::runtime_error(fmt::format("empty workspace not allowed"));
			
			// Validate projects: each of them has to be an object
			for(auto & projIt : it->items())
			{
				json &proj = projIt.value();
				if (proj.type() != jtype::object)
					throw std::runtime_error(fmt::format("each workspace project has to be an JSON object"));
				
				// Validate name and type
				expect(proj, "name", jtype::string);
				expect(proj, "type", jtype::string);
			}

			// Treat as workspace:
			auto name = root.find("name");
			if (name != root.end())
				expectType(*name, "name", jtype::string);
			else
			{
				// Use first project's name
				root["name"] = (*it)[0]["name"].get<std::string>();
			}

			// TODO: make each project conformant
		}
	}
	else 
		throw std::runtime_error(fmt::format("empty workspace not allowed"));			
}


}
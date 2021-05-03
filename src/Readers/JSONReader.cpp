#include PACC_PCH

#include <Pacc/Readers/JSONReader.hpp>

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
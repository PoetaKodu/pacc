#include PACC_PCH

#include <Pacc/Readers/JsonReader.hpp>


//////////////////////////////////////////////////
void PackageJsonReader::makeConformant()
{
	using JV 	= JsonView;
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
			JV{*it}.requireType("projects", jtype::array);

			// At least one project
			if (it->size() < 1)
				throw PaccException("empty workspace not allowed");
			
			// Validate projects: each of them has to be an object
			for(auto projIt : it->items())
			{
				json &proj = projIt.value();
				if (proj.type() != jtype::object)
					throw PaccException("each workspace project has to be an JSON object");
				
				// Validate name and type
				JV{proj}.expect("name", jtype::string);
				JV{proj}.expect("type", jtype::string);
			}

			// Treat as workspace:
			auto name = root.find("name");
			if (name != root.end())
				JV{*name}.requireType("name", jtype::string);
			else
			{
				// Use first project's name
				root["name"] = (*it)[0]["name"].get<std::string>();
			}

			// TODO: make each project conformant
		}
		else if (auto it = root.find("type");
			it != root.end())
		{
			if (!root.contains("name") || root["name"].type() != json::value_t::string)
				throw PaccException("Project doesn't contain string field \"name\".");

			json singleProject = std::move(root);

			root = json::object();
			root["name"] 		= singleProject["name"];
			root["version"]		= JV{singleProject}.stringFieldOr("version", "0.0.0");
			root["projects"] 	= json::array();
			root["projects"].push_back(std::move(singleProject));

			// It is in workspace format now, process it again.
			this->makeConformant();
		}
		else
			throw PaccException("Invalid cpackage.json format.")
				.withHelp("Insert either \"projects\" (workspace) or \"type\" (single project) field.");
	}
	else 
		throw PaccException("Empty workspace not allowed, your cpackage.json is invalid.");		
}
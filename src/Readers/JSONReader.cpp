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
				throw std::runtime_error(fmt::format("empty workspace not allowed"));
			
			// Validate projects: each of them has to be an object
			for(auto projIt : it->items())
			{
				json &proj = projIt.value();
				if (proj.type() != jtype::object)
					throw std::runtime_error(fmt::format("each workspace project has to be an JSON object"));
				
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
	}
	else 
		throw std::runtime_error(fmt::format("empty workspace not allowed"));			
}
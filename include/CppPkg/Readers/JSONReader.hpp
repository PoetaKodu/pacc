#pragma once

#include <CppPkg/Package.hpp>

#include <nlohmann/json.hpp>
#include <string_view>
#include <string>

using json = nlohmann::json;

namespace reader
{

class PackageJSONView
{
	static void expect(json &j, std::string_view name, json::value_t type);
	static void expectType(json const& j, std::string_view name, json::value_t type);
public:
	json& root;

	void makeConformant();
};

Package fromJSON(std::string const& packageContent_);

}
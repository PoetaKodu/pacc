#pragma once

#include PACC_PCH

#include <Pacc/Package.hpp>

namespace reader
{

class PackageJSONView
{
public:
	json& root;

	static void expect(json &j, std::string_view name, json::value_t type);
	static void expectType(json const& j, std::string_view name, json::value_t type);
	void makeConformant();
};

Package loadFromJSON(std::string const& packageContent_);

}
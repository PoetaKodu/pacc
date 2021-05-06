#pragma once

#include PACC_PCH

#include <Pacc/Package.hpp>

class PackageJSONView
{
public:
	json& root;

	static void expect(json &j, std::string_view name, json::value_t type);
	static void requireType(json const& j, std::string_view name, json::value_t type);
	void makeConformant();
};

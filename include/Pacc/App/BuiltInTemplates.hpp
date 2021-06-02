#pragma once

#include PACC_PCH

namespace app_tmpl
{

constexpr std::string_view cpackage_json =
R"PKG({{
	"$schema": "https://raw.githubusercontent.com/PoetaKodu/pacc/main/res/cpackage.schema.json",
	
	"name": "{PackageName}",
	"projects": [
		{{
			"name": "MyProject",
			"type": "app",
			"language": "C++17",
			"files": "src/*.cpp"
		}}
	]
}})PKG";

}
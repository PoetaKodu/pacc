#pragma once

#include PACC_PCH

namespace app_tmpl
{

constexpr StringView pacc_json =
R"PKG({{
	"$schema": "https://raw.githubusercontent.com/PoetaKodu/pacc/main/res/pacc.schema.json",

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

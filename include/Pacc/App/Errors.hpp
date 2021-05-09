#pragma once

#include PACC_PCH
namespace errors
{

// constexpr std::string_view NoPackageSourceFile =
// 	"No package source file inside current directory. "
// 	"\nPlease create one of the following:"
// 	"\n\tcpackage.lua \t\tpackage LUA script source file"
// 	"\n\tcpackage.json \t\tstatic package JSON build instructions";

constexpr std::string_view NoPackageSourceFile[2] = {
		// Error:
		"No package source file inside current directory. ",
		// Help
		"Please create one of the following:"
		"\n\tcpackage.lua \t\tpackage LUA script source file (NOT SUPPORTED YET)"
		"\n\tcpackage.json \t\tstatic package JSON build instructions"
	};


}
#pragma once

#include PACC_PCH
namespace errors
{

constexpr std::string_view NoPackageSourceFile[2] = {
		// Error:
		"No package file inside current directory.",
		// Help
		"Please create one of the following:"
		"\n\tpacc.json \t\tstatic package JSON build instructions"
		"\n\tpacc.lua \t\tpackage LUA script source file (NOT SUPPORTED YET)"
		"\n\tcpackage.lua \t\tpackage LUA script source file (NOT SUPPORTED YET, deprecated, will be removed in next major version)"
		"\n\tcpackage.json \t\tstatic package JSON build instructions (deprecated, will be removed in next major version)"
	};

}

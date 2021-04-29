#pragma once

#include <string_view>

namespace errors
{

constexpr std::string_view NoPackageSourceFile =
	"No package source file inside current directory. "
	"\nPlease create one of the following:"
	"\n\tcpackage.lua \t\tpackage LUA script source file"
	"\n\tcpackage.json \t\tstatic package JSON build instructions";

}
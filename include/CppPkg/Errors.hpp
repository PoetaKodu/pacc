#pragma once

#include <string_view>

namespace errors
{

constexpr std::string_view NoPackageSourceFile =
	"No package source file inside current directory. "
	"\nPlease create one of the following:"
	"\n\tpackage.lua \t\tpackage LUA script source file"
	"\n\tpackage.json \t\tstatic package JSON build instructions";

}
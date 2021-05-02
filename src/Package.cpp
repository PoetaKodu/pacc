#include <Blocc/Package.hpp>

#include <fmt/format.h>
#include <stdexcept>
#include <string>

///////////////////////////////////////////////////////////////
Dependency Dependency::from(std::string_view depPattern)
{
	if (depPattern.empty())
		throw std::runtime_error(fmt::format("Invalid dependency pattern \"{}\"", depPattern));

	auto pkgSep = depPattern.find(':');
	
	if (pkgSep != std::string::npos)
	{
		return Dependency{
				std::string(depPattern.substr(0, pkgSep)),
				std::string(depPattern.substr(pkgSep + 1))
			};	
	}
	return Dependency{ std::string(""), std::string(depPattern) };
}
#include "include/Pacc/PaccPCH.hpp"

#include <Pacc/Readers/General.hpp>
#include <Pacc/Helpers/Exceptions.hpp>

///////////////////////////////////////////////////
String readFileContents(fs::path const& path_)
{
	auto input = std::ifstream(path_);

	if (!input.is_open())
		throw PaccException("failed to open file {} for reading", path_.string());


	String result;
	String buf;
	buf.resize(4 * 1024 * 1024);
	while(input.read(buf.data(), buf.capacity()))
		result.append(buf.data(), input.gcount());
	result.append(buf.data(), input.gcount());

	return result;
}

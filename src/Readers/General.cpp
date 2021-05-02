#include BLOCC_PCH

#include <Blocc/Readers/General.hpp>

namespace reader
{

///////////////////////////////////////////////////
std::string readFileContents(fs::path const& path_)
{
	std::ifstream input(path_);

	if (!input.is_open())
		throw std::runtime_error(fmt::format("failed to open file {} for reading", path_.string()));
	
	std::string result;
	std::string buf;
	buf.reserve(4 * 1024 * 1024);
	while(input.read(buf.data(), buf.capacity()))
		result.append(buf.data(), input.gcount());
	result.append(buf.data(), input.gcount());

	return result;
}

}
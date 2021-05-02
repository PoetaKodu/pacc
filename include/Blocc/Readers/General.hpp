#pragma once

#include <string>
#include <filesystem>

namespace fs = std::filesystem;

namespace reader
{

std::string readFileContents(fs::path const& path_);


}
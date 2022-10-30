#pragma once

#include <Pacc/PaccPCH.hpp>

// File system extension
namespace fsx
{

// Convert backslashes to slashes
fs::path fwd(fs::path p_);

/// <summary>Tries to make files writable</summary>
/// <param name="path_">Target folder/file path</param>
void makeWritableAll(fs::path const& path_);

bool isJunction(fs::path const& path);
fs::path readJunction(fs::path const& path);

bool isSymlinkOrJunction(fs::path const& path);
fs::path readSymlinkOrJunction(fs::path const& path);


void createSymlink(fs::path const& target, fs::path const& link, bool junctionIfAvailable, std::error_code& err);
void createSymlink(fs::path const& target, fs::path const& link, bool junctionIfAvailable);


}

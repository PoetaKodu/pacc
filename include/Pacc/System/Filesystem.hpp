#pragma once

#include PACC_PCH

// File system extension
namespace fsx
{

// Convert backslashes to slashes
fs::path fwd(fs::path p_);

/// <summary>Tries to make files writable</summary>
/// <param name="path_">Target folder/file path</param>
void makeWritableAll(fs::path const& path_);

}
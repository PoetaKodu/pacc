#pragma once

#include PACC_PCH

namespace env
{

/// <summary>Returns path to the Pacc Data Storage Folder.</summary>
/// <returns>Path of the data storage.</returns>
/// <remarks>
/// 	On Windows: %AppData%/pacc
/// 	On Linux: %USER%/pacc
/// </remarks>
fs::path getPaccDataStorageFolder();

/// <summary>
/// 	Returns path to the Pacc Data Storage Folder.
/// 	Creates the folder if didn't exist.
/// </summary>
/// <returns>Path of the data storage.</returns>
fs::path requirePaccDataStorageFolder();

/// <summary>Finds executable with given name, that is visible from current folder (either in path, or same folder)</summary>
/// <param name="execName_">The executable name</param>
/// <returns>fs::path</returns>
fs::path findExecutable(std::string_view execName_);

/// <summary>
/// Gets the directory of the Pacc executable.
/// </summary>
fs::path getPaccAppPath();

}

#include PACC_PCH

#include <Pacc/System/Filesystem.hpp>

#ifdef PACC_SYSTEM_WINDOWS
	#include <Windows.h>
#endif

namespace fsx
{


//////////////////////////////////////
fs::path fwd(fs::path p_)
{
	// On windows replace backslashes with slashes
	#ifdef PACC_SYSTEM_WINDOWS
		std::wstring w = p_.wstring();
		rg::replace(w, L'\\', L'/');
		p_ = w;
	#endif

	return p_;
}

//////////////////////////////////////
void makeWritableAll(fs::path const& path_)
{
	if (!fs::exists(path_))
		return;

	if (fs::is_directory(path_))
	{
		for(auto entry : fs::recursive_directory_iterator(path_))
		{
			fs::permissions(entry.path(),
					fs::perms::owner_write | fs::perms::group_write,
					fs::perm_options::add
				);
		}
	}
	else
	{
		fs::permissions(path_,
				fs::perms::owner_write | fs::perms::group_write,
				fs::perm_options::add
			);
	}
}

#ifdef PACC_SYSTEM_WINDOWS
static bool isReparsePoint(fs::path const& path);
{
	auto attr = GetFileAttributesW(path.c_str());
	return (attr != INVALID_FILE_ATTRIBUTES) && (attr & FILE_ATTRIBUTE_REPARSE_POINT);
}
#endif

bool isJunction(fs::path const& path)
{
#ifdef PACC_SYSTEM_WINDOWS
	return fs::is_directory(path) && !fs::is_symlink(path) && isReparsePoint(path);
#endif
	return false;
}

#ifdef PACC_SYSTEM_WINDOWS

/// @brief Originally located in <ntifs.h>
/// but this requires Windows SDK to be installed.
/// Search MSDN for "REPARSE_DATA_BUFFER" for more info.
struct REPARSE_DATA_BUFFER
{
	ULONG ReparseTag;
	USHORT ReparseDataLength;
	USHORT Reserved;
	union
	{
		struct
		{
			USHORT SubstituteNameOffset;
			USHORT SubstituteNameLength;
			USHORT PrintNameOffset;
			USHORT PrintNameLength;
			ULONG Flags;
			WCHAR PathBuffer[1];
		} SymbolicLinkReparseBuffer;
		struct
		{
			USHORT SubstituteNameOffset;
			USHORT SubstituteNameLength;
			USHORT PrintNameOffset;
			USHORT PrintNameLength;
			WCHAR PathBuffer[1];
		} MountPointReparseBuffer;
		struct
		{
			UCHAR DataBuffer[1];
		} GenericReparseBuffer;
	} DUMMYUNIONNAME;
};
#endif

fs::path readJunction(fs::path const& path)
{
#ifdef PACC_SYSTEM_WINDOWS
	auto handle = CreateFileW(path.c_str(),
			GENERIC_READ,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			nullptr,
			OPEN_EXISTING,
			FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS,
			nullptr
		);
	if (handle == INVALID_HANDLE_VALUE)
		return fs::path();

	DWORD bytesReturned = 0;
	BYTE buffer[MAXIMUM_REPARSE_DATA_BUFFER_SIZE];
	auto result = DeviceIoControl(handle, FSCTL_GET_REPARSE_POINT, nullptr, 0, buffer, MAXIMUM_REPARSE_DATA_BUFFER_SIZE, &bytesReturned, nullptr);
	CloseHandle(handle);
	if (!result)
		return fs::path();

	auto reparseDataBuffer = reinterpret_cast<REPARSE_DATA_BUFFER*>(buffer);
	if (reparseDataBuffer->ReparseTag != IO_REPARSE_TAG_MOUNT_POINT)
		return fs::path();

	auto& mountPointBuf = reparseDataBuffer->MountPointReparseBuffer;
	auto printNameOffset = mountPointBuf.PrintNameOffset / sizeof(WCHAR);
	auto printNameLength = mountPointBuf.PrintNameLength / sizeof(WCHAR);

	auto printName = std::wstring(mountPointBuf.PathBuffer + printNameOffset, printNameLength);
	return fs::path(printName);
#else
	return fs::path();
#endif
}

void createSymlink(fs::path const& target, fs::path const& link, bool junctionIfAvailable, std::error_code& err)
{
#ifdef PACC_SYSTEM_WINDOWS
	if (junctionIfAvailable) {
		std::system(fmt::format("mklink /J \"{}\" \"{}\"", link.string(), target.string()).c_str());
		return;
	}
#endif
	fs::create_symlink(target, link, err);
}

void createSymlink(fs::path const& target, fs::path const& link, bool junctionIfAvailable)
{
#ifdef PACC_SYSTEM_WINDOWS
	if (junctionIfAvailable) {
		std::system(fmt::format("mklink /J \"{}\" \"{}\"", link.string(), target.string()).c_str());
		return;
	}
#endif
	fs::create_symlink(target, link);
}

bool isSymlinkOrJunction(fs::path const& path) {
	return fs::is_symlink(path) || isJunction(path);
}

fs::path readSymlinkOrJunction(fs::path const& path)
{
	if (fs::is_symlink(path))
		return fs::read_symlink(path);
	else if (isJunction(path))
		return readJunction(path);

	return fs::path();
}

}

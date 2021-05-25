#include PACC_PCH

#include <Pacc/System/Filesystem.hpp>

namespace fsx
{


//////////////////////////////////////
fs::path fwd(fs::path p_)
{
	// On windows replace backslashes with slashes 
	#ifdef PACC_SYSTEM_WINDOWS
		std::wstring w = p_.wstring();
		std::replace(w.begin(), w.end(), L'\\', L'/');
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

}
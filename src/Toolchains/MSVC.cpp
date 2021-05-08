#include PACC_PCH

#include <Pacc/Toolchains/MSVC.hpp>
#include <Pacc/System/Process.hpp>

///////////////////////////////////////////////
std::vector<MSVCToolchain> MSVCToolchain::detect()
{
	// TODO: find better way to find this program
	// TODO: this won't support older visual studios
	const std::string vswherePath 	= "C:/Program Files (x86)/Microsoft Visual Studio/Installer/vswhere";
	const std::string params 		= " -prerelease -sort -format json -utf8";

	std::vector<MSVCToolchain> tcs;

	ChildProcess vswhere{vswherePath + params};
	auto exitCode = vswhere.runSync();

	if (exitCode.value_or(1) == 0)
	{
		using jt = json::value_t;
		json j = json::parse(vswhere.out.stdOut);

		if (j.type() == jt::array)
		{
			for(auto tcDescIt : j.items())
			{
				auto const& tcDesc = tcDescIt.value();

				MSVCToolchain tc;
				tc.prettyName 	= tcDesc["displayName"].get<std::string>();
				tc.version 		= tcDesc["catalog"]["productDisplayVersion"].get<std::string>();
				tc.mainPath 	= tcDesc["installationPath"].get<std::string>();
				tcs.push_back(std::move(tc));
			}
		}
	}

	return tcs;
}
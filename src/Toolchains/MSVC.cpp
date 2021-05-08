#include PACC_PCH

#include <Pacc/Toolchains/MSVC.hpp>
#include <Pacc/System/Process.hpp>
#include <Pacc/PackageSystem/Package.hpp>

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

///////////////////////////////
std::optional<int> MSVCToolchain::run(Package const& pkg_)
{
	using fmt::fg, fmt::color;

	fmt::print(fg(color::gray), "Running MSBuild... ");

	// TODO: make configurable
	std::string_view params[] = {
		"/m",
		"/property:Configuration=Debug",
		"/property:Platform=x64",
		// Ask msbuild to generate full paths for file names.
		"/property:GenerateFullPaths=true",
		"/t:build"
	};

	// TODO: Maybe make configurable?
	fs::path const msbuildPath = mainPath / "MSBuild/Current/Bin/msbuild.exe";

	std::string buildCommand = fmt::format("{} {}.sln", msbuildPath.string(), pkg_.name);
	for(auto p : params)
		buildCommand += fmt::format(" \"{}\"", p);

	return ChildProcess{buildCommand, "build", 30}.runSync();
}
#include PACC_PCH

#include <Pacc/Toolchains/MSVC.hpp>
#include <Pacc/System/Process.hpp>
#include <Pacc/Generation/Logs.hpp>
#include <Pacc/PackageSystem/Package.hpp>

#include <ranges>


///////////////////////////////////////////////
static auto detectVSProperty(std::string propertyName)
{
	// TODO: find better way to find this program
	// TODO: this won't support older visual studios
	const std::string vswherePath 		= "C:/Program Files (x86)/Microsoft Visual Studio/Installer/vswhere";
	const std::string params 			= "-prerelease -sort -utf8 -property ";

	auto result = std::vector<std::string>();

	ChildProcess vswhere{ fmt::format("\"{}\" {} {}", vswherePath, params, propertyName), "", ch::milliseconds{2500} };
	auto exitCode = vswhere.runSync();
	if (exitCode.value_or(1) != 0)
		return std::optional( result );

	std::erase(vswhere.out.stdOut, '\r');
	for (auto strRange : std::views::split(vswhere.out.stdOut, '\n'))
	{
		auto common = strRange | std::views::common;
		auto str = std::string(common.begin(), common.end());
		if (str.empty())
			continue;

		result.emplace_back( std::move(str) );
	}

	return std::optional( result );
}

///////////////////////////////////////////////
std::vector<MSVCToolchain> MSVCToolchain::detect()
{
	auto tcs = std::vector<MSVCToolchain>();

	// Read pretty names
	{
		auto displayNames = detectVSProperty("displayName");

		if (!displayNames.has_value())
			return tcs;

		tcs.reserve(displayNames->size());

		for (auto& elem : *displayNames)
		{
			MSVCToolchain tc;
			tc.prettyName = std::move(elem);
			tcs.emplace_back(std::move(tc));
		}
	}

	// Read versions
	{
		auto versions = detectVSProperty("catalog.productDisplayVersion");

		if (versions.has_value())
		{
			for (size_t i = 0; i < versions->size(); ++i)
				tcs[i].version = std::move( (*versions)[i] );
		}
	}

	// Read line versions
	{
		auto lineVersions = detectVSProperty("catalog.productLineVersion");

		if (lineVersions.has_value())
		{
			for (size_t i = 0; i < lineVersions->size(); ++i)
				tcs[i].lineVersion = parseLineVersion((*lineVersions)[i]);
		}
	}

	// Read installation paths
	{
		auto paths = detectVSProperty("installationPath");

		if (paths.has_value())
		{
			for (size_t i = 0; i < paths->size(); ++i)
				tcs[i].mainPath = std::move( (*paths)[i] );
		}
	}

	return tcs;
}

///////////////////////////////
std::string MSVCToolchain::handleWin32SpecialCase(std::string const& platformName_)
{
	if (platformName_ == "x86")
		return "Win32";

	return platformName_;
}

///////////////////////////////
MSVCToolchain::LineVersion MSVCToolchain::parseLineVersion(std::string const& lvStr_)
{
	LineVersion lv;
	try {
		lv = static_cast<LineVersion>(std::stoi(lvStr_));
	} catch(...) {}

	return lv;
}


///////////////////////////////
std::optional<int> MSVCToolchain::run(Package const& pkg_, BuildSettings settings_, int verbosityLevel_)
{
	using fmt::fg, fmt::color;

	bool verbose = (verbosityLevel_ > 0);

	fmt::print(fg(color::gray), "Running MSBuild... {}", verbose ? "\n" : "");


	// TODO: make configurable
	std::vector<std::string> params = {
		"/m",
		"/property:Configuration=" + settings_.configName,
		"/property:Platform=" + handleWin32SpecialCase(settings_.platformName),
		// Ask msbuild to generate full paths for file names.
		"/property:GenerateFullPaths=true"
	};

	if (settings_.targetName.empty())
		params.push_back("/t:build");
	else
	{
		params.push_back("/t:" + settings_.targetName);
		params.push_back("/p:BuildProjectReferences=false");
	}

	if (settings_.cores.has_value())
		params.push_back(fmt::format("/p:CL_MPCount={}", settings_.cores.value()));

	fs::path const msbuildPath = mainPath / "MSBuild/Current/Bin/msbuild.exe";

	std::string buildCommand = fmt::format("{} {}.sln", msbuildPath.string(), pkg_.name);
	for(auto p : params)
		buildCommand += fmt::format(" \"{}\"", p);

	ChildProcess proc{buildCommand, "build", std::nullopt, verbose};
	proc.runSync();

	std::string outputLog = fmt::format(
			FMT_COMPILE("STDOUT:\n\n{}\n\nSTDERR:\n\n{}"),
			proc.out.stdOut,
			proc.out.stdErr
		);

	saveBuildOutputLog(pkg_.name, outputLog);

	return proc.exitCode;
}

///////////////////////////////
void MSVCToolchain::serialize(json& out_) const
{
	Toolchain::serialize(out_);

	out_["lineVersion"] = static_cast<uint32_t>(lineVersion);
}

///////////////////////////////
bool MSVCToolchain::deserialize(json const& in_)
{
	if(!Toolchain::deserialize(in_))
		return false;

	auto it = in_.find("lineVersion");
	if (it == in_.end() || it->type() != json::value_t::number_unsigned)
		return false;

	lineVersion = static_cast<LineVersion>(it->get<int>());

	return true;
}

///////////////////////////////
std::string MSVCToolchain::premakeToolchainType() const
{
	switch(lineVersion)
	{
	case LineVersion::VS2022: return "vs2022";
	case LineVersion::VS2019: return "vs2019";
	case LineVersion::VS2017: return "vs2017";
	case LineVersion::VS2015: return "vs2015";
	case LineVersion::VS2013: return "vs2013";
	default: return "vs2019"; // not found
	}
}

#include PACC_PCH

#include <Pacc/Plugins/CMake.hpp>

#include <Pacc/System/Process.hpp>
#include <Pacc/Helpers/Exceptions.hpp>
#include <Pacc/Readers/General.hpp>
#include <Pacc/App/App.hpp>

namespace plugins::cmake
{

constexpr StringView TargetHashSeparator		= "::@";
constexpr StringView CacheProjectVersionKey	= "CMAKE_PROJECT_VERSION:STATIC=";

///////////////////////////////////////
static inline ProjectType projectTypeFromString(String const& str_)
{
	if(str_ == "EXECUTABLE")
		return ProjectType::App;
	else if(str_ == "STATIC_LIBRARY")
		return ProjectType::StaticLib;
	else if(str_ == "SHARED_LIBRARY")
		return ProjectType::SharedLib;
	else if(str_ == "UTILITY")
		return ProjectType::Unknown;
	else
		throw PaccException("Unknown/unsupported CMake project type: {}", str_);
}

///////////////////////////////////////
static inline auto buildFolderOf(fs::path const& path_)
{
	return path_ / "build";
}

///////////////////////////////////////
static inline auto queryFolderOf(fs::path const& packagePath_)
{
	return buildFolderOf(packagePath_) / ".cmake" / "api" / "v1" / "query";
}

///////////////////////////////////////
static inline auto replyFolderOf(fs::path const& packagePath_)
{
	return buildFolderOf(packagePath_) / ".cmake" / "api" / "v1" / "reply";
}

///////////////////////////////////////
void createQueryFile(fs::path const& packagePath_)
{
	// Ensure folders exist
	auto targetFolder = queryFolderOf(packagePath_);
	fs::create_directories(targetFolder);

	// Create an empty file
	std::ofstream(targetFolder / "codemodel-v2") << "";
}

///////////////////////////////////////
json readReplyFile(fs::path const& root_)
{
	auto replyFolder = replyFolderOf(root_);
	// find reply file
	fs::path replyFile;
	for (auto entry : fs::directory_iterator(replyFolder))
	{
		if (entry.is_regular_file()) {
			auto path = entry.path().filename().wstring();
			if (path.starts_with(L"codemodel-v2")) {
				replyFile = entry.path();
				break;
			}
		}
	}

	if (replyFile.empty())
		return {};

	return json::parse(readFileContents(replyFile));
}


///////////////////////////////////////
auto runCMakeCommand(fs::path const& packagePath_, StringView command_)
{
	String generator = "Visual Studio 17 2022";
	auto command = fmt::format("cmake -G=\"{}\" {} ..", generator, command_);
	auto proc = ChildProcess{
			command,
			packagePath_ / "build", ch::seconds{2 * 60}
		};

	fmt::print("Running command: {}\n", command);
	proc.printRealTime = true;
	return proc.runSync();
}

///////////////////////////////////////
auto runCMakeBuildCommand(fs::path const& packagePath_, StringView commandOpt_ = "")
{
	auto command = fmt::format("cmake --build .", commandOpt_);
	auto proc = ChildProcess{
			command,
			packagePath_ / "build", ch::seconds{15 * 60}
		};

	fmt::print("Running build command: {}\n", command);
	proc.printRealTime = true;
	return proc.runSync();
}

///////////////////////////////////////
BuildInfo runBuildInfoQuery(fs::path const& packagePath_)
{
	BuildInfo result;

	createQueryFile(packagePath_);

	auto cmakeResult = runCMakeCommand(packagePath_, " -D BUILD_SHARED_LIBS=0 -D CMAKE_BUILD_TYPE=Debug");
	if (!cmakeResult.has_value() || cmakeResult.value() != 0)
		throw PaccException("CMake build info query failed");

	return result;
}

///////////////////////////////////////
UPtr<Package> PackageLoader::load(fs::path const& root_)
{
	runBuildInfoQuery(root_);

	auto reply		= readReplyFile(root_);
	auto targets	= this->discoverTargets(reply);

	auto package = std::make_unique<Package>();
	package->root = root_;
	package->outputRoot = "build";
	package->builder = app.packageBuilders["cmake"].get();
	package->version = this->loadVersion(root_);

	fmt::print("Package version: {}\n", package->version.toString());

	for (auto const&[name, path] : targets)
	{
		Project p;
		this->loadProjectFromFile(replyFolderOf(root_) / path, p);
		package->projects.push_back(std::move(p));
	}

	return package; // TODO: move loading logic from Package class
}

///////////////////////////////////////
bool PackageLoader::loadTarget(fs::path const& root_, String const& name_, TargetBase& target_)
{
	// TODO: implement this
	return false;
}

///////////////////////////////////////
Vec< StringPair > PackageLoader::discoverTargets(json const& json_) const
{
	Vec< StringPair > result;
	result.reserve(16);

	auto confs = json_["configurations"];

	for (auto conf : confs)
	{
		if (conf.value("name", "") == "Debug")
		{
			for (auto target : conf["targets"])
			{
				auto id = target.value("id", "");
				if (id.find(TargetHashSeparator) != String::npos)
				{
					result.push_back( { std::move(id), target.value("jsonFile", "") } );
				}
			}
		}
	}

	return result;
}


///////////////////////////////////////
bool PackageLoader::loadProjectFromFile(fs::path const& file_, Project& project_)
{
	auto json = json::parse(readFileContents(file_));

	project_.name = json.value("name", "");
	project_.type = projectTypeFromString(json.value("type", "STATIC_LIBRARY"));

	if (json.contains("artifacts"))
	{
		for (auto artifact : json["artifacts"].items())
		{
			auto path = artifact.value().value("path", "");
			if (!path.empty())
			{
				// TODO: detect artifact type from path
				Artifact artType = detectArtifactTypeFromPath(path);

				project_.artifacts[(size_t)artType].push_back(path);
				fmt::print("Project {} (type: {}) -> {}\n", project_.name, toString(project_.type), path);
			}
		}
	}

	return true;
}

///////////////////////////////////////
Version PackageLoader::loadVersion(fs::path const& root_)
{
	// read cmake cache
	auto cacheFile = buildFolderOf(root_) / "CMakeCache.txt";
	if (!fs::exists(cacheFile))
		return {};

	auto cache = readFileContents(cacheFile);
	auto pos = cache.find(CacheProjectVersionKey);
	if (pos == String::npos)
		return {};

	auto endOfLine = cache.find('\n', pos);
	auto view = StringView(cache);
	return Version::fromString(view.substr(pos + CacheProjectVersionKey.size(), endOfLine - pos - CacheProjectVersionKey.size()));
}

///////////////////////////////////////
BuildProcessResult PackageBuilder::run(Package const& pkg_, Toolchain& tc_, BuildSettings const& settings_, int verbosityLevel_)
{
	return runCMakeBuildCommand(pkg_.root, "--config Debug");
}

}

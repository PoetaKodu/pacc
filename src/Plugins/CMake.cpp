#include PACC_PCH

#include <Pacc/Plugins/CMake.hpp>

#include <Pacc/System/Process.hpp>
#include <Pacc/Helpers/Exceptions.hpp>
#include <Pacc/Readers/General.hpp>


namespace plugins::cmake
{

///////////////////////////////////////
static inline auto queryFolderOf(fs::path const& packagePath_)
{
	return packagePath_ / "build" / ".cmake" / "api" / "v1" / "query";
}

///////////////////////////////////////
static inline auto replyFolderOf(fs::path const& packagePath_)
{
	return packagePath_ / "build" / ".cmake" / "api" / "v1" / "reply";
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
auto runCMakeCommand(fs::path const& packagePath_, std::string_view command_)
{
	std::string generator = "Visual Studio 17 2022";
	auto command = fmt::format("cmake -G=\"{}\" {} ..", generator, command_);
	ChildProcess makeVer{
			command,
			packagePath_ / "build", ch::seconds{2 * 60}
		};

	fmt::print("Running command: {}\n", command);
	makeVer.printRealTime = true;
	return makeVer.runSync();
}

///////////////////////////////////////
BuildInfo runBuildInfoQuery(fs::path const& packagePath_)
{
	BuildInfo result;

	createQueryFile(packagePath_);

	auto cmakeResult = runCMakeCommand(packagePath_, " -D BUILD_SHARED_LIBS=0 -D CMAKE_BUILD_TYPE=Debug");
	if (!cmakeResult.has_value() || cmakeResult.value() != 0)
		throw PaccException("CMake build info query failed");

	auto replyFolder = replyFolderOf(packagePath_);

	for (auto entry : fs::directory_iterator(replyFolder))
	{
		if (entry.is_regular_file()) {
			auto path = entry.path().filename().wstring();
			if (path.starts_with(L"target-")) {
				auto json = json::parse(readFileContents(entry.path()));
				auto name = json["name"];
				if (!name.is_null()) {
					fmt::print("Target: {}\n", name.get<std::string>());
				}
				auto artifacts = json["artifacts"];

				for (auto art : artifacts) {
					auto artPath = art["path"];
					if (artPath != nullptr) {
						fmt::print("{:4}- {}\n", ' ', artPath.get<std::string>());
					}
				}
			}
			// else {
			// 	std::wcout << path << L'\n';
			// }
		}
	}

	return result;
}

///////////////////////////////////////
UPtr<Package> PackageLoader::load(fs::path const& root_)
{
	runBuildInfoQuery(root_);
	TargetBase unused;
	this->loadTarget(root_, "sfml-system", unused);
	this->loadTarget(root_, "sfml-audio", unused);
	this->loadTarget(root_, "sfml-window", unused);
	this->loadTarget(root_, "sfml-graphics", unused);

	return Package::load(root_); // TODO: move loading logic from Package class
}

///////////////////////////////////////
bool PackageLoader::loadTarget(fs::path const& root_, std::string const& name_, TargetBase& target_)
{
	auto json = readReplyFile(root_);

	// read targets
	auto confs = json["configurations"];

	for (auto conf : confs)
	{
		if (conf.value("name", "") == "Debug")
		{
			for (auto target : conf["targets"])
			{
				if (target.value("id", "").starts_with(name_ + "::@"))
				{
					fmt::print("Target {} found in file {}\n", name_, target.value("jsonFile", ""));
					return true;
				}
			}
		}
	}

	return false;
}

}

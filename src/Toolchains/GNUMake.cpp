#include "include/Pacc/PaccPCH.hpp"

#include <Pacc/Toolchains/GNUMake.hpp>
#include <Pacc/Helpers/Exceptions.hpp>
#include <Pacc/Helpers/String.hpp>
#include <Pacc/System/Environment.hpp>
#include <Pacc/System/Process.hpp>
#include <Pacc/Generation/Logs.hpp>
#include <Pacc/PackageSystem/Package.hpp>

///////////////////////////////////////////////
Vec<GNUMakeToolchain> GNUMakeToolchain::detect()
{
	fs::path makePath = env::findExecutable("make");

	Vec<GNUMakeToolchain> tcs;

	if (!makePath.empty() && fs::exists(makePath))
	{
		// Make show version command:
		// make -v
		String command = makePath.string() + " -v";

		auto makeVer = ChildProcess{command, "", ch::milliseconds{2500}};

		auto exitStatus = makeVer.runSync();
		if (exitStatus.value_or(1) == 0)
		{
			// Example output:
			// GNU Make 4.2.0
			// <some other lines>

			// Method: parse first line.
			// TODO: find better method if available.

			String& stdOut = makeVer.out.stdOut;

			String firstLine;

			// Remove all lines but first:
			{
				size_t newLinePos = stdOut.find_first_of("\r\n");
				if (newLinePos != String::npos)
					firstLine = stdOut.substr(0, newLinePos);
			}

			GNUMakeToolchain tc;
			tc.mainPath = makePath.parent_path();

			// Parse version:
			{
				// Find version digit:
				size_t digitPos = firstLine.find_first_of("0123456789");
				if (digitPos != String::npos)
				{
					tc.prettyName 	= firstLine.substr(0, digitPos - 1); // -1 because of the space
					tc.version 		= firstLine.substr(digitPos);
				}
				else
					throw PaccException("Could not parse make name and version from string \"{}\"", firstLine);
			}


			tcs.push_back(std::move(tc));
		}
	}

	return tcs;
}


///////////////////////////////
Opt<int> GNUMakeToolchain::run(Package const & pkg_, BuildSettings settings_, int verbosityLevel_)
{
	using fmt::fg, fmt::color;

	bool verbose = (verbosityLevel_ > 0);

	fmt::print(fg(color::gray), "Running GNU Make... {}", verbose ? "\n" : "");

	Vec<String> params =
		{
			// Note: this probably won't work on configurations with spaces in names
			fmt::format("config={}_{}",
					toLower(settings_.configName),
					toLower(settings_.platformName)
				),
			fmt::format("CXX={}", cppCompilerName),
			fmt::format("CC={}", cCompilerName)
		};

	if (settings_.cores.has_value())
		params.push_back(fmt::format("-j{}", settings_.cores.value()));

	String buildCommand = (mainPath / "make").string();
	for(auto p : params)
		buildCommand += fmt::format(" \"{}\"", p);

	auto proc = ChildProcess{buildCommand, "build", std::nullopt, verbose};

	proc.runSync();

	String outputLog = fmt::format(
			FMT_COMPILE("STDOUT:\n\n{}\n\nSTDERR:\n\n{}"),
			proc.out.stdOut,
			proc.out.stdErr
		);
	saveBuildOutputLog(pkg_.name, outputLog);

	return proc.exitCode;
}

////////////////////////////////////////////
bool GNUMakeToolchain::isEqual(Toolchain const& other_) const
{
	if (!Toolchain::isEqual(other_))
		return false;

	auto otherAsMake = dynamic_cast<GNUMakeToolchain const*>(&other_);
	if (!otherAsMake)
		return false;

	return (otherAsMake->cppCompilerName == this->cppCompilerName
		&& otherAsMake->cCompilerName == this->cCompilerName);
}

///////////////////////////////
void GNUMakeToolchain::serialize(json& out_) const
{
	Toolchain::serialize(out_);

	out_["cppCompiler"] = this->cppCompilerName;
	out_["cCompiler"] 	= this->cCompilerName;
}

///////////////////////////////
bool GNUMakeToolchain::deserialize(json const& in_)
{
	if (!Toolchain::deserialize(in_))
		return false;

	using JV = JsonView;
	auto view = JsonView{in_};

	cppCompilerName	= view.stringFieldOr("cppCompiler",	"g++");
	cCompilerName	= view.stringFieldOr("cCompiler",	"gcc");

	return true;
}

#include PACC_PCH

#include <Pacc/Toolchains/GNUMake.hpp>
#include <Pacc/Helpers/Exceptions.hpp>
#include <Pacc/Helpers/String.hpp>
#include <Pacc/System/Process.hpp>
#include <Pacc/Generation/Logs.hpp>
#include <Pacc/PackageSystem/Package.hpp>

///////////////////////////////////////////////
fs::path GNUMakeToolchain::findMake()
{
	const std::string command =
	#ifdef PACC_SYSTEM_WINDOWS
		"where make";
	#else
		"type -a -P make";
	#endif

	// TODO:
	ChildProcess make{command, "", ch::milliseconds{500}};
	auto exitStatus = make.runSync();

	if (exitStatus.value_or(1) == 0)
	{
		std::string& stdOut = make.out.stdOut;

		fs::path makePath;

		// Parse `where make` output
		size_t newLinePos = make.out.stdOut.find_first_of("\r\n");
		if (newLinePos != std::string::npos)
			return stdOut.substr(0, newLinePos);
		else
			return stdOut;
	}

	return {};
}

///////////////////////////////////////////////
std::vector<GNUMakeToolchain> GNUMakeToolchain::detect()
{
	fs::path makePath = findMake();

	std::vector<GNUMakeToolchain> tcs;

	if (!makePath.empty() && fs::exists(makePath))
	{
		// Make show version command:
		// make -v 
		std::string command = makePath.string() + " -v";

		ChildProcess makeVer{command, "", ch::milliseconds{500}};

		auto exitStatus = makeVer.runSync();
		if (exitStatus.value_or(1) == 0)
		{
			// Example output:
			// GNU Make 4.2.0
			// <some other lines>

			// Method: parse first line.
			// TODO: find better method if available.

			std::string& stdOut = makeVer.out.stdOut;

			std::string firstLine;

			// Remove all lines but first:
			{
				size_t newLinePos = stdOut.find_first_of("\r\n");
				if (newLinePos != std::string::npos)
					firstLine = stdOut.substr(0, newLinePos);
			}

			GNUMakeToolchain tc;
			tc.mainPath = makePath.parent_path();

			// Parse version:
			{
				// Find version digit:
				size_t digitPos = firstLine.find_first_of("0123456789");
				if (digitPos != std::string::npos)
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
std::optional<int> GNUMakeToolchain::run(Package const & pkg_, BuildSettings settings_, int verbosityLevel_)
{
	using fmt::fg, fmt::color;

	bool verbose = (verbosityLevel_ > 0);

	fmt::print(fg(color::gray), "Running GNU Make... {}", verbose ? "\n" : "");

	std::string params[] =
		{
			// Note: this probably won't work on configurations with spaces in names
			fmt::format("config={}_{}",
					to_lower(settings_.configName),
					to_lower(settings_.platformName)
				)
		};

	std::string buildCommand = (mainPath / "make").string();
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
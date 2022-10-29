#include PACC_PCH

#include <Pacc/App/App.hpp>

///////////////////////////////////
void PaccApp::query()
{
	constexpr StringView OutputSwitches[] = { "-o", "--output" };
	auto outFileName = String();

	for (auto sw : OutputSwitches)
	{
		auto val = this->argValue(sw);
		if (!val.empty()) {
			outFileName = std::move(val);
			break;
		}
	}

	auto outFile = std::ofstream{};
	if (!outFileName.empty())
	{
		outFile.open(outFileName);
		if (!outFile.is_open())
		{
			throw PaccException("Cannot open file \"{}\" for writing.", outFileName)
				.withHelp("Ensure that you specified the right file with \"-o\" or \"--output\" switch.");
		}
	}

	if (args.size() < 2)
	{
		throw PaccException("No query type.")
			.withHelp("Use \"pacc query package\" to query a package.");
	}


	auto& out = outFile.is_open() ? outFile : std::cout;


	auto pkg = Package::load();

	json j;
	j["name"]		= pkg->name;
	j["projects"]	= json::array();

	for (auto const& p : pkg->projects)
	{
		json proj;
		proj["name"] = p.name;
		proj["type"] = toString(p.type);
		j["projects"].push_back(std::move(proj));
	}

	out << j.dump(1, '\t');
}

#include PACC_PCH

#include <Pacc/App/App.hpp>
#include <Pacc/Visualization/Graph.hpp>


///////////////////////////////////
void PaccApp::visualizeGraph()
{
	constexpr StringView OutputSwitches[] = { "-o", "--output" };

	auto pkg = Package::load();

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

	auto& out = outFile.is_open() ? outFile : std::cout;

	out << viz::generateGraphContent(*pkg);
}

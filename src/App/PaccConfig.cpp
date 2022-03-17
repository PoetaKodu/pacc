#include PACC_PCH

#include <Pacc/App/PaccConfig.hpp>
#include <Pacc/Readers/General.hpp>
#include <Pacc/Helpers/Json.hpp>

#include <Pacc/Toolchains/MSVC.hpp>
#include <Pacc/Toolchains/GNUMake.hpp>

/////////////////////////////////////////////////
PaccConfig PaccConfig::loadOrCreate(fs::path const& jsonPath_)
{
	if (!fs::exists(jsonPath_))
	{
		fs::create_directories(jsonPath_.parent_path());
		std::ofstream{ jsonPath_ } << "{\n\t\n}";

		PaccConfig emptyConfig;
		emptyConfig.path = jsonPath_;
		return emptyConfig;
	}

	return load(jsonPath_);
}

/////////////////////////////////////////////////
PaccConfig PaccConfig::load(fs::path const& jsonPath_)
{
	PaccConfig result;
	result.path = jsonPath_;

	json j = json::parse(readFileContents(jsonPath_));

	result.detectedToolchains = result.readToolchains(j, "detectedToolchains");
	auto customTcs = result.readToolchains(j, "customToolchains");
	result.toolchains = result.detectedToolchains; // copy the toolchains

	result.toolchains.insert(result.toolchains.end(), customTcs.begin(), customTcs.end());

	result.readSelectedToolchain(j);
	
	return result;
}

/////////////////////////////////////////////////
void PaccConfig::readSelectedToolchain(json const& input_)
{
	bool invalid = true;
	auto it = input_.find("selectedToolchain");

	if (it != input_.end() && it->type() == json::value_t::number_unsigned)
	{
		int idx = it->get<int>();
		if (idx >= 0 && idx < toolchains.size())
		{
			this->selectedToolchain = idx;
			invalid = false;
		}
	}

	if (invalid)
	{
		this->updateSelectedToolchain(0);
	}
}

/////////////////////////////////////////////////
PaccConfig::VecOfTc PaccConfig::readToolchains(json const& input_, std::string const& field_)
{
	using JV = JsonView;

	VecOfTc result;

	auto it = input_.find(field_);

	if (it == input_.end() || it->type() != json::value_t::array)
		return result;

	for(auto jsonTcIt : it->items())
	{
		auto& jsonTc = jsonTcIt.value();

		if (jsonTc.type() != json::value_t::object)
			continue;
		
		std::string tcType = JV{jsonTc}.stringFieldOr("type", "");

		UPtr<Toolchain> tc;

		if (tcType.empty())
			continue;
		else if (tcType == "msvc")
			tc = std::make_unique<MSVCToolchain>();
		else if (tcType == "gnumake")
			tc = std::make_unique<GNUMakeToolchain>();

		if (tc && tc->deserialize(jsonTc))
		{
			result.emplace_back( std::move(tc) );
		}
	}

	return result;
}


/////////////////////////////////////////////////
bool PaccConfig::ensureValidToolchains(Vec< SPtr<Toolchain> > & current_)
{
	if (!this->validateDetectedToolchains(current_))
	{
		selectedToolchain = 0;

		this->updateToolchains(std::move(current_));
		return true;
	}

	return false;
}

/////////////////////////////////////////////////
void PaccConfig::updateToolchains(Vec< SPtr<Toolchain> > current_)
{
	if (!detectedToolchains.empty())
		toolchains.erase(toolchains.begin(), toolchains.begin() + detectedToolchains.size());

	detectedToolchains = std::move(current_);
	toolchains.insert(toolchains.begin(), detectedToolchains.begin(), detectedToolchains.end());

	json j = json::parse(readFileContents(path));

	j["detectedToolchains"] = serializeToolchains(detectedToolchains);
	

	std::ofstream(path) << j.dump(1, '\t');
}

/////////////////////////////////////////////////
void PaccConfig::updateSelectedToolchain(int index_)
{
	// Unchecked!

	selectedToolchain = index_;

	json j = json::parse(readFileContents(path));

	j["selectedToolchain"] = int(selectedToolchain);
	
	std::ofstream(path) << j.dump(1, '\t');
}

/////////////////////////////////////////////////
bool PaccConfig::validateDetectedToolchains(Vec< SPtr<Toolchain> > const& current_) const
{
	if (detectedToolchains.size() != current_.size())
		return false;

	for(size_t i = 0; i < detectedToolchains.size(); ++i)
	{
		if (!detectedToolchains[i]->isEqual(*current_[i]))
			return false;
	}

	return true;
}

/////////////////////////////////////////////////
json PaccConfig::serializeToolchains(VecOfTc const& tcs_)
{
	std::vector<json> result;
	result.reserve(tcs_.size());

	for(auto const& tc : tcs_)
	{
		json j = json::object();
		tc->serialize(j);

		result.push_back(std::move(j));
	}

	return result;
}
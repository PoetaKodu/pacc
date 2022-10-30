#include "include/Pacc/PaccPCH.hpp"

#include <Pacc/Toolchains/Toolchain.hpp>

////////////////////////////////////////////
StringView Toolchain::typeName(Type type_)
{
	switch(type_)
	{
		case MSVC: 		return "msvc";
		case GNUMake: 	return "gnumake";
		default: 		return "unknown";
	}
}

////////////////////////////////////////////
Opt<int> Toolchain::run(struct Package const & pkg_, BuildSettings settings_, int verbosityLevel_)
{
	return 1;
}

////////////////////////////////////////////
String Toolchain::premakeToolchainType() const
{
	return "";
}

////////////////////////////////////////////
Toolchain::Type Toolchain::type() const
{
	return Unknown;
}

////////////////////////////////////////////
bool Toolchain::generateProjectFiles()
{
	fmt::printErr(fmt::runtime("Not implemented."));
	return false;
}

////////////////////////////////////////////
bool Toolchain::isEqual(Toolchain const& other_) const
{
	if (other_.type() != this->type())
		return false;

	return (
			prettyName 	== other_.prettyName 	&&
			version 	== other_.version 		&&
			mainPath 	== other_.mainPath
		);
}

////////////////////////////////////////////
void Toolchain::serialize(json& out_) const
{
	out_["prettyName"] 	= prettyName;
	out_["version"] 	= version;

	{
		// Note: this is a workaround, because std::u8string is detected as an array of numbers by json
		auto mp = mainPath.u8string();
		out_["mainPath"] = String(mp.begin(), mp.end());
	}
	out_["type"] 		= Toolchain::typeName(this->type());
}

////////////////////////////////////////////
bool Toolchain::deserialize(json const& in_)
{
	using JV = JsonView;
	auto view = JsonView{in_};

	prettyName 	= view.stringFieldOr("prettyName", 	"Unknown");
	version 	= view.stringFieldOr("version", 	"?.?.?");
	mainPath 	= view.stringFieldOr("mainPath", 	"");

	if (mainPath == "")
		return false;

	return true;
}

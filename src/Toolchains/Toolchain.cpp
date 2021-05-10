#include PACC_PCH

#include <Pacc/Toolchains/Toolchain.hpp>

////////////////////////////////////////////
std::string_view Toolchain::typeName(Type type_)
{
	switch(type_)
	{
		case MSVC: 		return "msvc";
		case GNUMake: 	return "gnumake";
		default: 		return "unknown";
	}
}

////////////////////////////////////////////
std::optional<int> Toolchain::run(struct Package const & pkg_, BuildSettings settings_, int verbosityLevel_)
{
	return 1;
}

////////////////////////////////////////////
std::string Toolchain::premakeToolchainType() const
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
	fmt::printErr("Not implemented.");
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
	out_["mainPath"] 	= mainPath.u8string();
	out_["type"] 		= Toolchain::typeName(this->type());
}

////////////////////////////////////////////
bool Toolchain::deserialize(json const& in_)
{
	using JV = JsonView;
	JsonView view{in_};

	prettyName 	= view.stringFieldOr("prettyName", 	"Unknown");
	version 	= view.stringFieldOr("version", 	"?.?.?");
	mainPath 	= view.stringFieldOr("mainPath", 	"");

	if (mainPath == "")
		return false;

	return true;
}
#include "include/Pacc/PaccPCH.hpp"


#include <Pacc/App/App.hpp>
#include <Pacc/App/BuiltInTemplates.hpp>

////////////////// FORWARD DECLARATIONS

static bool ensureUserApproval();
static void writeStringToFile(fs::path const& outputFile_, String const& what_);
static void getInitPackageNameAndTarget(ProgramArgs const& args_, fs::path& target_, String& targetName_);

///////////////////////////////////////////////////
void PaccApp::initPackage()
{
	using fmt::fg, fmt::color;
	using namespace fmt::literals;

	auto cwd		= fs::current_path();
	auto target		= fs::path();
	auto targetName	= String();

	getInitPackageNameAndTarget(args, target, targetName);

	fmt::print( "Initializing package \"{}\"\n"
				"Do you want to create \"pacc.json\" file (Y/N): ",
				target.stem().string() );

	if (!ensureUserApproval())
		return;

	fs::create_directories(target);

	writeStringToFile(target / "pacc.json",
			fmt::format(app_tmpl::pacc_json,
				"PackageName"_a = target.stem().string()
			)
		);

	fmt::print(fg(color::lime_green),
			"\"pacc.json\" has been created.\n"
			"Happy development!"
		);
}

///////////////////////////////////////////////////
static void writeStringToFile(fs::path const& outputFile_, String const& what_)
{
	std::ofstream(outputFile_) << what_;
}

///////////////////////////////////////////////////
static bool ensureUserApproval()
{
	String response;
	std::getline(std::cin, response);

	if (response[0] != 'y' && response[0] != 'Y')
	{
		std::cout << "Action aborted." << std::endl;
		return false;
	}
	return true;
}

///////////////////////////////////////////////////
static void getInitPackageNameAndTarget(ProgramArgs const& args_, fs::path& target_, String& targetName_)
{
	target_ 	= fs::current_path();
	targetName_	= target_.stem().string();

	if (args_.size() > 2 && args_[2] != ".")
	{
		targetName_ = args_[2];

		if (fs::path(targetName_).is_relative())
			target_ /= targetName_;
		else
			target_ = targetName_;


		auto packageFile = findPackageFile(target_);
		if (!packageFile.empty())
		{
			throw PaccException("Folder \"{}\" already contains {}!", targetName_, packageFile.filename().string());
		}
	}
}

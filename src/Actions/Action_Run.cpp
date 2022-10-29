#include PACC_PCH

#include <Pacc/App/App.hpp>

#include <Pacc/System/Filesystem.hpp>
#include <Pacc/System/Process.hpp>

///////////////////////////////////////////////////
void PaccApp::run()
{
	auto pkg = Package::load();

	if (pkg->projects.empty())
		throw PaccException("Package \"{}\" does not contain any projects.", pkg->name);

	auto settings = this->determineBuildSettingsFromArgs();

	Project const* project = nullptr;

	String targetName;
	// Try to get target name from args (ignore build settings --target flag, too complex)
	if (args.size() >= 3 && args[2][0] != '-') // not a switch
		targetName = args[2];

	if (targetName.empty())
		targetName = pkg->startupProject;

	if (targetName.empty())
	{
		for(auto const &proj : pkg->projects)
		{
			if (proj.type == Project::Type::App)
			{
				project = &pkg->projects[0];
				break;
			}
		}
		if (!project)
			throw PaccException("Package \"{}\" does not contain any runnable projects.", pkg->name);
	}
	else
	{
		project = pkg->findProject(targetName);
		if (!project)
			throw PaccException("Package \"{}\" does not contain project with name \"{}\".", pkg->name, targetName);
	}

	if (project->type != Project::Type::App)
	{
		throw PaccException("Cannot run project \"{}\", because it's not an application (type: \"{}\", expected: \"app\").", project->name, project->type)
			.withHelp("If the package contains other application projects, use \"pacc run [project_name]\"");
	}

	fs::path outputFile = fsx::fwd(pkg->predictRealOutputFolder(*project, settings) / project->name);

	#ifdef PACC_SYSTEM_WINDOWS
	outputFile += ".exe";
	#endif

	if (!fs::exists(outputFile))
		throw PaccException("Could not find project \"{}\" binary.", project->name)
			.withHelp("Use \"pacc build\" command first and make sure it succeeded.");

	auto before = ch::steady_clock::now();

	auto exitStatus = ChildProcess{outputFile.string(), "", std::nullopt, true}.runSync();

	auto dur = ch::duration_cast< ch::duration<double> >(ch::steady_clock::now() - before);

	fmt::print("\nProgram ended after {:.2f}s with {} exit status.\n", dur.count(), exitStatus.value_or(1));
}

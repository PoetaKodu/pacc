#include "include/Pacc/PaccPCH.hpp"

#include <Pacc/App/App.hpp>

#include <Pacc/PackageSystem/Package.hpp>
#include <Pacc/App/Errors.hpp>
#include <Pacc/PackageSystem/Events.hpp>
#include <Pacc/System/Environment.hpp>
#include <Pacc/Readers/General.hpp>
#include <Pacc/System/Filesystem.hpp>
#include <Pacc/Readers/JsonReader.hpp>
#include <Pacc/Generation/BuildQueueBuilder.hpp>

#include <Pacc/Plugins/CMake.hpp>


///////////////////////////////////////////////////
// Private functions (forward declaration)
///////////////////////////////////////////////////

template <json::value_t type>
auto expect(json const& j) -> json const*;

template <json::value_t type>
auto expectSub(json const& j, StringView subfieldName) -> json const*;

template <json::value_t type>
auto require(json const& j) -> json const&;

template <json::value_t type>
auto requireSub(json const& j, StringView subfieldName) -> json const&;

auto selfOrSubfieldOpt(json const& self, StringView fieldName = "") -> json const*;
auto selfOrSubfieldReq(json const& self, StringView fieldName = "") -> json const&;
auto selfOrSubfield(json const& self, StringView fieldName, bool required = false) -> json const*;

auto readDependencyAccess(Package &pkg_, Project & proj_, json const& deps_, Vec<Dependency> &target_) -> void;
auto loadVecOfStrField(json const& j, StringView fieldName, bool direct = false, bool required = false) -> Vec<String>;
auto loadVecOfStrAccField(json const& j, StringView fieldName, AccessType defaultAccess_ = AccessType::Private) -> VecOfStrAcc;

void loadConfigurationFromJSON(Package & pkg_, Project & project_, Configuration& conf_, json const& root_);


void readSingleTargetEventHandler(
		PaccAppModule_EventHandlerActions const&	events_,
		EventHandlingTarget&						target_,
		String const&								key,
		json const&									value,
		int											handlerIndex = 0
	)
{
	if (value.is_string())
	{
		auto action = value.get<String>(); // TODO: ensure is a string
		auto [name, behavior] = splitBy(action, ':', true);

		if (action.empty())
		{
			// TODO: consider throwing an error
			return;
		}

		auto actionHandler = events_.findEventAction(name);
		if (!actionHandler)
		{
			fmt::printLog(
					fmt::fg(fmt::color::yellow),
					"[Warning] Handler (id: {}) of the event \"{}\" for target \"{}\" uses unknown action \"{}\":\n    ",
					handlerIndex, key, target_.name, name
				);
			fmt::printLog(fmt::bg(fmt::color::dark_red), "{}", name);
			fmt::printLog(fmt::fg(fmt::color::dim_gray), ":{}\n\n", behavior);
			return;
		}

		if (!actionHandler->canLoadFromAbbreviatedString)
		{
			fmt::printLog(
					fmt::fg(fmt::color::yellow),
					"Action \"{}\" cannot be loaded from an abbreviated string. Use a JSON object instead.\n",
					key, target_.name
				);
			return;
		}

		target_.eventHandlers[key].push_back(actionHandler->load(behavior));
	}
	else if (value.is_object())
	{
		auto action = value.value<String>("action", "");

		if (action.empty())
		{
			fmt::printLog(
					fmt::fg(fmt::color::yellow),
					"[Warning] Handler (id: {}) of the event \"{}\" for target \"{}\" has no \"action\" field specified.\n",
					handlerIndex, key, target_.name
				);
			// TODO: consider throwing an error
			return;
		}

		auto actionHandler = events_.findEventAction(action);
		if (!actionHandler)
		{
			fmt::printLog(
					fmt::fg(fmt::color::yellow),
					"[Warning] Handler (id: {}) of the event \"{}\" for target \"{}\" uses unknown action \"{}\":\n    ",
					handlerIndex, key, target_.name, action
				);
			fmt::printLog(fmt::fg(fmt::color::gray), "\"action\": \"");
			fmt::printLog(fmt::bg(fmt::color::dark_red), "{}", action);
			fmt::printLog(fmt::fg(fmt::color::gray), "\"\n\n");
			return;
		}

		target_.eventHandlers[key].push_back(actionHandler->load(value));
	}
}

// TODO: refactor
void readTargetEventHandlers(
		PaccAppModule_EventHandlerActions const&	events_,
		json const&									scriptsContainer,
		EventHandlingTarget&						target_
	)
{
	if (!scriptsContainer.contains("events"))
		return;
	auto scripts = scriptsContainer["events"];

	for (auto [key, value] : scripts.items())
	{
		if (value.is_array())
		{
			auto idx = 0;
			for (auto const& item : value)
			{
				readSingleTargetEventHandler(events_, target_, key, item, idx);
				++idx;
			}
		}
		else readSingleTargetEventHandler(events_, target_, key, value);
	}
}



///////////////////////////////////////////////////
// Public functions
///////////////////////////////////////////////////


///////////////////////////////////////////////////
auto findPackageFile(Path const& directory_, Opt<StringView> extension_)
	-> Path
{
	auto fileExists = [&] (Path const& path_)
	{
		auto fullPath = directory_ / path_;
		return fs::exists(fullPath) && fs::is_regular_file(fullPath);
	};

	// Test JSON files:
	{
		auto it = rg::find_if(PackageJSON, fileExists);
		if (it != std::end(PackageJSON))
			return directory_ / *it;
	}

	// Test LUA files:
	{
		auto it = rg::find_if(PackageLUA, fileExists);
		if (it != std::end(PackageLUA))
			return directory_ / *it;
	}

	return {};
}

///////////////////////////////////////////////////
auto findPackageScriptFile(Path const& directory_)
	-> Path
{
	auto fileExists = [&] (Path const& path_)
	{
		auto fullPath = directory_ / path_;
		return fs::exists(fullPath) && fs::is_regular_file(fullPath);
	};

	auto it = rg::find_if(PackageLUAScript, fileExists);
	if (it != std::end(PackageLUAScript))
		return directory_ / *it;

	return {};
}

///////////////////////////////////////////////////
auto detectArtifactTypeFromPath(StringView path_)
	-> Artifact
{
	if (path_.ends_with(".lib"))
		return Artifact::LibraryInterface;
	else if (path_.ends_with(".dll") || path_.ends_with(".so") || path_.ends_with(".a"))
		return Artifact::Library;
	else if (path_.ends_with(".exe"))
		return Artifact::Executable;
	else if (path_.ends_with(".pdb"))
		return Artifact::DebugSymbols;

	return Artifact::Unknown;
}

///////////////////////////////////////////////////
void TargetBase::inheritConfigurationFrom(Package const& fromPkg_, Project const& fromProject_, AccessType mode_)
{
	{
		// First: if the "fromProject" is a library, add it as a dependency
		if (fromProject_.type == Project::Type::StaticLib || fromProject_.type == Project::Type::SharedLib)
		{
			// Add dependency output folder:
			{
				auto& target = targetByAccessType(linkerFolders.computed, mode_);
				target.push_back(fsx::fwd(fromPkg_.predictOutputFolder(fromProject_)).string());
			}

			// Add dependency file to linker:
			{
				auto& target = targetByAccessType(linkedLibraries.computed, mode_);
				target.push_back(fromProject_.outputArtifact().string());
			}
		}
	}

	computeConfiguration( *this, fromPkg_, fromProject_, mode_ );


	// Inherit all premake filters:
	for(auto it : fromProject_.premakeFilters)
	{
		// Ensure configuration exists:
		if (premakeFilters.find(it.first) == premakeFilters.end())
			premakeFilters[it.first] = {};

		// Merge configuration:
		computeConfiguration( premakeFilters.at(it.first), fromPkg_, fromProject_, it.second, mode_ );
	}
}

///////////////////////////////////////////////////
auto TargetBase::outputArtifact() const
	-> Path
{
	if (!artifacts[(size_t)Artifact::Executable].empty())
	{
		return artifacts[(size_t)Artifact::Executable].front().filename();
	}
	if (!artifacts[(size_t)Artifact::Library].empty())
	{
		return artifacts[(size_t)Artifact::Library].front().filename();
	}
	if (!artifacts[(size_t)Artifact::LibraryInterface].empty())
	{
		return artifacts[(size_t)Artifact::LibraryInterface].front().filename();
	}

	return name;
}

///////////////////////////////////////////////////
auto toString(ProjectType type_, StringView pluginName_)
	-> String
{
	switch (type_)
	{
		case ProjectType::App:
			return "app";
		case ProjectType::StaticLib:
			return "static lib";
		case ProjectType::SharedLib:
			return "shared lib";
		case ProjectType::Interface:
			return "interface";
		case ProjectType::HandledByPlugin:
			return fmt::format("plugin:{}", pluginName_);
		default:
			return "unknown";
	}
}

///////////////////////////////////////////////////
auto parseProjectType(StringView type_)
	-> ProjectType
{
	if (compareIgnoreCase(type_, "app"))
		return Project::Type::App;
	else if (compareIgnoreCase(type_, "static lib"))
		return Project::Type::StaticLib;
	else if (compareIgnoreCase(type_, "shared lib"))
		return Project::Type::SharedLib;
	else if (compareIgnoreCase(type_, "interface"))
		return Project::Type::Interface;
	else if (compareIgnoreCase(type_.substr(0, 7), "plugin:"))
		return Project::Type::HandledByPlugin;
	else
		return Project::Type::Unknown;

	return ProjectType::Unknown;
}

///////////////////////////////////////////////////
auto Project::isLibrary() const -> bool
{
	return type == Type::StaticLib || type == Type::SharedLib;
}

///////////////////////////////////////////////////
auto Project::getPrimaryArtifactOfType(Artifact artType_) const
	-> Path
{
	auto& outputs = artifacts[(size_t)artType_];
	if (outputs.empty())
		return {};
	return outputs[0];
}

///////////////////////////////////////////////////
auto Project::getLinkTargetArtifact() const
	-> Path
{
	if (type == ProjectType::SharedLib || type == ProjectType::StaticLib)
	{
		auto art = getPrimaryArtifactOfType(Artifact::LibraryInterface);
		if (art.empty())
			art = getPrimaryArtifactOfType(Artifact::Library);
		return art;
	}
	else
		return {};
}

///////////////////////////////////////////////////
auto Project::getPrimaryArtifact() const
	-> Path
{
	switch (type)
	{
	case ProjectType::App:
		return getPrimaryArtifactOfType(Artifact::Executable);
	case ProjectType::StaticLib:
	case ProjectType::SharedLib:
		return getLinkTargetArtifact();
	default:
		return {};
	}
}

///////////////////////////////////////////////////
void Package::loadPackageSpecificInfo(json const& json_)
{
	name 			= json_["name"].get<String>();
	startupProject	= json_.value("startupProject", "");
	version 		= Version::fromString( json_.value("version", "0") );

	if (json_.contains("cmake"))
		isCMake = json_.value("cmake", false);
	else
		isCMake = false;

	readTargetEventHandlers(useApp(), json_, *this);
}

///////////////////////////////////////////////////
void Package::loadWorkspaceInfo(json const& json_)
{
	using json_vt = json::value_t;

	auto projectsNode = json_.find("projects");

	// projects.reserve(projectsNode->size());

	// Read projects:
	for(auto it : projectsNode->items())
	{
		auto& jsonProject = it.value();

		Project project;

		project.name = jsonProject["name"].get<String>();
		project.type = parseProjectType(jsonProject["type"].get<String>());

		readTargetEventHandlers(useApp(), jsonProject, project);

		if (auto it = jsonProject.find("pch"); it != jsonProject.end())
		{
			PrecompiledHeader pch;
			// TODO: add validation
			pch.header		= jsonProject["pch"]["header"];
			pch.source		= jsonProject["pch"]["source"];
			pch.definition 	= jsonProject["pch"]["definition"];
			project.pch = std::move(pch);
		}

		// TODO: type and value validation
		if (auto it = jsonProject.find("language"); it != jsonProject.end())
			project.language = it->get<String>();

		loadConfigurationFromJSON(*this, project, project, jsonProject);

		json const* filters = expectSub<json_vt::object>(jsonProject, "filters");
		if (filters)
		{
			for(auto filterIt : filters->items())
			{
				auto const& val = filterIt.value();
				if (val.type() == json_vt::object)
				{
					// Create and reference the configuration:
					Configuration& cfg = project.premakeFilters[filterIt.key()];
					loadConfigurationFromJSON(*this, project, cfg, val);
				}
			}
		}

		projects.emplace_back(std::move(project));
	}

	if (isCMake) {
		plugins::cmake::runBuildInfoQuery(root.parent_path());
	}
}

///////////////////////////////////////////////////
auto Package::preload(Path dir_)
	-> PackagePreloadInfo
{
	PackagePreloadInfo result;

	if (dir_.empty()) {
		dir_ = fs::current_path();
	}

	result.root = findPackageFile(dir_);
	if (result.root.empty())
	{
		throw PaccException(errors::NoPackageSourceFile[0])
				.withHelp(errors::NoPackageSourceFile[1]);
	}

	result.scriptFile = findPackageScriptFile(dir_);

	return result;
}


///////////////////////////////////////////////////
auto Package::load(PackagePreloadInfo preloadInfo_)
	-> UPtr<Package>
{
	auto pkg = UPtr<Package>();

	// Decide what to do:
	if (preloadInfo_.usesJsonConfig())
	{
		pkg = std::make_unique<Package>();
		pkg->root		= std::move(preloadInfo_.root);
		pkg->scriptFile	= std::move(preloadInfo_.scriptFile);

		Package::loadFromJSON(*pkg, readFileContents(pkg->root));
	}
	else // Lua config
	{
		// TODO: implement this.
		throw PaccException("This function is not implemented yet.")
				.withHelp("Use \"pacc.json\" config for now.");
	}
	return pkg;
}


///////////////////////////////////////////////////
auto Package::findProject(StringView name_) const
	-> Project const*
{
	auto it = rg::find(projects, name_, &Project::name);

	if (it != projects.end())
		return &(*it);

	return nullptr;
}

///////////////////////////////////////////////////
auto Package::requireProject(StringView name_) const
	-> Project const&
{
	Project const *proj = this->findProject(name_);
	if (!proj)
		throw PaccException("Project \"{}\" does not exist in package \"{}\"", name_, name);

	return *proj;
}


///////////////////////////////////////////////////
auto Package::predictOutputFolder(Project const& project_) const
	-> Path
{
	auto artifact = project_.getLinkTargetArtifact();

	if (!artifact.empty())
	{
		return (this->outputRoot.is_absolute() ? this->outputRoot : this->rootFolder() / this->outputRoot) / artifact.parent_path();
	}


	// TODO: make it configurable:
	return this->root.parent_path() / "bin/%{cfg.platform}/%{cfg.buildcfg}";
}

///////////////////////////////////////////////////
auto Package::predictRealOutputFolder(Project const& project_, BuildSettings settings_) const
	-> Path
{
	auto folder = Path();

	auto artifact = project_.getLinkTargetArtifact();
	if (!artifact.empty())
	{
		folder = artifact.parent_path();
	}
	else {
		folder = fmt::format("bin/{}/{}",
			settings_.platformName,
			settings_.configName
		);
	}

	return (this->outputRoot.is_absolute() ? this->outputRoot : this->rootFolder() / this->outputRoot) / folder;
}

///////////////////////////////////////////////////
auto Package::getAbsoluteArtifactFilePath(Project const& project_, BuildSettings settings_) const
	-> Path
{
	return this->predictRealOutputFolder(project_, settings_) / project_.getPrimaryArtifact().filename();
}

///////////////////////////////////////////////////
auto Package::resolvePath( Path const& path_) const
	-> Path
{
	if (path_.is_relative())
		return fsx::fwd(root.parent_path() / path_).string();
	else
		return path_;
}

///////////////////////////////////////////////////
void loadConfigurationFromJSON(Package & pkg_, Project & project_, Configuration& conf_, json const& root_)
{
	using fmt::fg, fmt::color;
	using json_vt = json::value_t;

	auto jv = JsonView{root_};

	conf_.symbolVisibility 		= GNUSymbolVisibility::fromString(root_.value("symbolVisibility", "Default"));
	conf_.moduleDefinitionFile 	= root_.value("moduleDefinitionFile", "");

	bool isInterface = (project_.type == Project::Type::Interface);

	AccessType defaultAccess = isInterface ? AccessType::Interface : AccessType::Private;

	conf_.files		 			= loadVecOfStrField(root_, "files");
	conf_.defines.self	 		= loadVecOfStrAccField(root_, "defines", 			defaultAccess);
	conf_.includeFolders.self	= loadVecOfStrAccField(root_, "includeFolders",		defaultAccess);
	conf_.linkerFolders.self	= loadVecOfStrAccField(root_, "linkerFolders", 		defaultAccess);
	conf_.compilerOptions.self	= loadVecOfStrAccField(root_, "compilerOptions", 	defaultAccess);
	conf_.linkerOptions.self	= loadVecOfStrAccField(root_, "linkerOptions", 		defaultAccess);

	// Load dependencies:
	auto depsIt = root_.find("dependencies");
	if (depsIt != root_.end())
	{
		auto& deps = depsIt.value();
		auto& projSelfDeps = conf_.dependencies.self;
		if (deps.type() == json_vt::array)
		{
			readDependencyAccess(pkg_, project_, *depsIt, targetByAccessType(projSelfDeps, defaultAccess));
		}
		else if (deps.type() == json_vt::object)
		{
			if (isInterface)
			{
				if (deps.contains("public") || deps.contains("private"))
					fmt::print(fg(color::yellow), "Interface project \"{}\" cannot include public or private dependencies (ignored).", project_.name);
			}
			else
			{
				if (deps.contains("public")) readDependencyAccess(pkg_, project_, deps["public"], projSelfDeps.public_);
				if (deps.contains("private")) readDependencyAccess(pkg_, project_, deps["private"], projSelfDeps.private_);
			}

			if (deps.contains("interface")) readDependencyAccess(pkg_, project_, deps["interface"], projSelfDeps.interface_);
		}
		else
			throw PaccException("Invalid type of \"dependencies\" field (must be an array or an object)");
	}
}

///////////////////////////////////////////////////
auto Package::loadFromJSON(Package& package_, String const& packageContent_)
	-> bool
{
	using json_vt = json::value_t;

	// Parse and make conformant:
	json j;
	auto view = PackageJsonReader{ j };

	j = json::parse(packageContent_);
	view.makeConformant();

	// Load JSON:
	package_.loadPackageSpecificInfo(j);
	package_.loadWorkspaceInfo(j);

	return true;
}

/////////////////////////////////////////////////
auto getNumElements(Vec<String> const& v)
	-> std::size_t
{
	return v.size();
}

/////////////////////////////////////////////////
auto getNumElements(VecOfStrAcc const& v)
	-> std::size_t
{
	return v.public_.size() + v.private_.size() + v.interface_.size();
}


/////////////////////////////////////////////////
void computeConfiguration(Configuration& into_, Package const& fromPkg_, Project const& fromProject_, Configuration const& from_, AccessType mode_)
{
	auto resolvePath = [&](auto const& pathLikeElem)
		{
			return fromPkg_.resolvePath(Path(pathLikeElem)).string();
		};

	mergeAccesses(into_.defines, 			from_.defines, 		 		mode_);
	mergeAccesses(into_.includeFolders, 	from_.includeFolders,  		mode_, resolvePath);
	mergeAccesses(into_.linkerFolders, 		from_.linkerFolders,  		mode_, resolvePath);
	mergeAccesses(into_.linkedLibraries, 	from_.linkedLibraries, 		mode_);
	mergeAccesses(into_.compilerOptions, 	from_.compilerOptions, 		mode_);
	mergeAccesses(into_.linkerOptions, 		from_.linkerOptions, 		mode_);
}

///////////////////////////////////////////////////
// Private functions
///////////////////////////////////////////////////

///////////////////////////////////////////////////
void readDependencyAccess(Package &pkg_, Project & proj_, json const& deps_, Vec<Dependency> &target_)
{
	using json_vt = json::value_t;

	if (deps_.type() != json_vt::array)
		throw PaccException("invalid type of dependencies subfield - array required");

	target_.reserve(deps_.size());

	for(auto item : deps_.items())
	{
		if (json const* rawDep = expect<json_vt::string>(item.value()))
		{
			String depPattern = rawDep->get<String>();
			if (startsWith(depPattern, "file:"))
			{
				target_.push_back( Dependency::raw( depPattern.substr(5) ) );
			}
			else if (startsWith(depPattern, "self:"))
			{
				auto sd = SelfDependency { &proj_, depPattern.substr(5), &pkg_ };
				target_.push_back( Dependency::self( std::move(sd) ) );
			}
			else
			{
				DownloadLocation loc = DownloadLocation::parse(depPattern);

				PackageDependency pd;
				pd.packageName 		= loc.repository;

				try {
					pd.version = VersionReq::fromString(loc.branch);
				}
				catch(...) {} // just ignore

				pd.projects.push_back(loc.repository);
				pd.downloadLocation = std::move(depPattern);

				target_.push_back(
						Dependency::package( std::move(pd) )
					);
			}
		}
		else if (json const* pkgDep = expect<json_vt::object>(item.value()))
		{
			// Required fields:
			json const& name		= requireSub<json_vt::string>(*pkgDep, "name");
			json const* projects	= expectSub<json_vt::array>(*pkgDep, "projects");
			// Optional fields:
			json const* version		= expectSub<json_vt::string>(*pkgDep, "version");

			// Configure dependency:
			PackageDependency pd;

			// Required:
			pd.packageName = name;

			// Parse download location:
			pd.downloadLocation = pkgDep->value("from", "");
			auto loc = DownloadLocation::parse(pd.downloadLocation);

			if (projects)
			{
				pd.projects.reserve(projects->size());
				for(auto proj : projects->items())
				{
					json const& projName = require<json_vt::string>(proj.value());

					pd.projects.push_back(projName.get<String>());
				}
			}
			else
			{
				String originalName;

				if (!loc.repository.empty())
					originalName = loc.repository;
				else
					originalName = name;

				pd.projects.emplace_back(std::move(originalName));
			}

			// Optional
			if (version)
			{
				try {
					pd.version = VersionReq::fromString(version->get<String>());
				}
				catch (...) {
					pd.version.type = VersionReq::Any;
				}
			}

			target_.push_back(
					Dependency::package( std::move(pd) )
				);
		}
		else
			throw PaccException("Invalid dependency type");
	}


}


///////////////////////////////////////////////////
auto selfOrSubfieldOpt(json const &self, StringView fieldName)
	-> json const*
{
	if (fieldName == "")
		return &self;
	else
	{
		if (auto it = self.find(fieldName); it != self.end())
			return &it.value();
	}

	return nullptr;
}

///////////////////////////////////////////////////
auto selfOrSubfieldReq(json const &self, StringView fieldName)
	-> json const&
{
	json const* v = selfOrSubfieldOpt(self, fieldName);
	if (!v)
		throw PaccException("field {0} not found", fieldName);
	else
		return *v;
}

///////////////////////////////////////////////////
auto selfOrSubfield(json const &self, StringView fieldName, bool required)
	-> json const*
{
	if (required)
		return &selfOrSubfieldReq(self, fieldName);
	else
		return selfOrSubfieldOpt(self, fieldName);
}

///////////////////////////////////////////////////
auto loadVecOfStrField(json const &j, StringView fieldName, bool direct, bool required)
	-> Vec<String>
{
	using JV = JsonView;

	Vec<String> result;
	String const elemName = String(fieldName) + " element";

	// Either subfield or the `j` itself (direct => `j` is an array)
	json const* val = selfOrSubfield(j, direct ? "" : fieldName, required);

	// Can be null if `required` == false
	if (!val)
		return result;

	if (val->type() == json::value_t::string)
	{
		result.push_back(*val);
	}
	else
	{
		JV(*val).requireType(fieldName, json::value_t::array);

		// Read the array:
		result.reserve(val->size());

		for(auto elem : val->items())
		{
			JV{elem.value()}.requireType(elemName, json::value_t::string);
			result.push_back(elem.value());
		}
	}
	return result;
}

///////////////////////////////////////////////////
auto loadVecOfStrAccField(json const &j, StringView fieldName, AccessType defaultAccess_)
	-> VecOfStrAcc
{
	VecOfStrAcc result;
	if (auto it = j.find(fieldName); it != j.end())
	{
		json::value_t type = it.value().type();
		if (type == json::value_t::array || type == json::value_t::string)
			targetByAccessType(result, defaultAccess_) = loadVecOfStrField(*it, fieldName, true);
		else
		{
			result.private_ 	= loadVecOfStrField(*it, "private");
			result.public_ 		= loadVecOfStrField(*it, "public");
			result.interface_ 	= loadVecOfStrField(*it, "interface");
		}
	}
	return result;
}

///////////////////////////////////////////////////
template <json::value_t type>
auto expect(json const &j)
	-> json const*
{
	if (j.type() == type)
		return &j;
	else
		return nullptr;
}

///////////////////////////////////////////////////
template <json::value_t type>
auto expectSub(json const &j, StringView subfieldName)
	-> json const*
{
	auto it = j.find(subfieldName);
	if (it != j.end() && it->type() == type)
	{
		return (&(*it));
	}

	return nullptr;
}

///////////////////////////////////////////////////
template <json::value_t type>
auto require(json const &j)
	-> json const&
{
	if (j.type() == type)
		return j;
	else
		throw PaccException("invalid type");
}

///////////////////////////////////////////////////
template <json::value_t type>
auto requireSub(json const &j, StringView subfieldName)
	-> json const&
{
	auto it = j.find(subfieldName);
	if (it != j.end() && it->type() == type)
	{
		return (*it);
	}

	throw PaccException("invalid subfield type");
}

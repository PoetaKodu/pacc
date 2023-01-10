#pragma once

#include <Pacc/PaccPCH.hpp>
#include <Pacc/Helpers/HelperTypes.hpp>

struct PaccMainAction {
	enum Type {
		None = 0,
		Version,
		Help,
		Init,
		Generate,
		Build,
		Link,
		Unlink,
		Logs,
		Install,
		Uninstall,
		ListVersions,
		ListPackages,
		Toolchains,
		Run,
		Graph,
		Query,
	} type = None;

	PaccMainAction() = default;
	PaccMainAction(Type type_) : type(type_) {}

	operator Type() const {
		return type;
	}

	static auto fromString(StringView str) -> PaccMainAction {
		if (str == "version") return Version;
		if (str == "help") return Help;
		if (str == "init") return Init;
		if (str == "generate") return Generate;
		if (str == "build") return Build;
		if (str == "link") return Link;
		if (str == "unlink") return Unlink;
		if (str == "logs" || str == "log") return Logs;
		if (str == "install" || str == "i") return Install;
		if (str == "uninstall") return Uninstall;
		if (str == "list-versions" || str == "list-version" || str == "lsver") return ListVersions;
		if (str == "list-packages" || str == "list" || str == "ls") return ListPackages;
		if (str == "toolchains" || str == "toolchain" || str == "tc") return Toolchains;
		if (str == "run") return Run;
		if (str == "graph") return Graph;
		if (str == "query") return Query;
		return None;
	}
};

struct ProgramFlag
{
	StringView	name;
	StringView	value;

	bool canBeMultiIndex = false;

	/// @brief  The index of the first program argument that is part of this flag
	size_t startIndex = 0;

	/// @brief  The index of the last program argument that is part of this flag
	size_t endIndex = 0;

	auto hasValue() const -> bool {
		return !value.empty();
	}

	auto isSet() const -> bool {
		return value != "" && value != "false" && value != "0";
	}


	/// @brief Returns true if the flag takes only one program argument, false otherwise
	/// @example
	/// # Returns true for:
	/// --flag
	/// --flag=false
	/// "--flag false"
	/// # Returns false for:
	/// --flag value
	auto isSingleIndex() -> bool {
		return startIndex == endIndex;
	}
};

using ProgramFlagMap = UMap<String, SPtr<ProgramFlag>>;


struct RunSettings
{
	/// The main action that was parsed
	PaccMainAction mainAction = PaccMainAction::None;

	/// The index of the name of the action that was parsed, even if invalid. 0 means invalid
	size_t actionNameIndex = 0;

	/// See `fromArgs()` implementation to view the list
	ProgramFlagMap flags;

	/// Indices of program arguments that were parsed - main actions have to respect that
	Vec<size_t> parsedArgs;

	bool wasParsed(size_t programArgIdx) const;
	bool isFlagSet(String const& name) const {
		auto it = flags.find(name);
		return it != flags.end() && it->second->isSet();
	}

	template <typename T>
	auto tryGetFlagValue(String const& name) const -> Opt<T> {
		auto it = flags.find(name);
		if (it == flags.end()) return {};
		return tryParse<T>(it->second->value);
	}

	/// @brief Returns the index of the nth not parsed argument after the argument
	/// that was parsed as the main action
	auto nthActionArgument(size_t n) const -> Opt<size_t>;

	static auto fromArgs(Vec<String> const& args_) -> RunSettings;

private:
	// Pointer to the program arguments
	Vec<String> const* args;
};


#include PACC_PCH

#include <Pacc/PackageSystem/Dependency.hpp>

/////////////////////////////////////////////////
DownloadLocation DownloadLocation::parse(std::string const& depTemplate_)
{
	if (depTemplate_.empty())
		return {};
		
	DownloadLocation result;

	std::size_t colonPos = depTemplate_.find(':');
	std::string rest;
	std::string platformName;

	if (colonPos != std::string::npos)
	{
		rest = depTemplate_.substr( colonPos + 1 );

		platformName = toLower(depTemplate_.substr(0, colonPos));

		if (platformName == "github")
			result.platform = GitHub;
		else if (platformName == "gitlab")
			result.platform = GitLab;
	}
	else
	{
		rest = depTemplate_;

		result.platform = OfficialRepo;
	}

	std::string repo;
	if (result.platform == OfficialRepo)
	{
		repo = std::move(rest);
	}
	else
	{
		std::size_t slashPos = rest.find('/');
		if (slashPos == std::string::npos)
		{
			throw PaccException("Invalid package \"{}\". Unknown user name.", depTemplate_)
				.withHelp("Use following syntax: \"{}:UserName/RepoName\"\n", platformName);
		}

		result.userName 	= rest.substr(0, slashPos);

		repo = rest.substr(slashPos + 1);
	}

	auto repoAndBranch = splitBy(repo, '@');

	result.repository 	= repoAndBranch.first;
	result.branch 		= repoAndBranch.second;
	if (!result.branch.empty())
	{
		result.exactBranch = result.branch[0] == '!';

		if (result.exactBranch) // exact branch
			result.branch = result.branch.substr(1);		
	}

	return result;
}


///////////////////////////////////////
std::string DownloadLocation::getGitLink() const
{
	if (platform == DownloadLocation::Unknown)
		return "";

	std::string platformName;
	std::string user = userName;
	switch(platform)
	{
	case DownloadLocation::OfficialRepo:
	{
		user = "pacc-repo";
		[[fallthrough]];
	}
	case DownloadLocation::GitHub:
	{
		platformName = "github";
		break;
	}
	case DownloadLocation::GitLab:
	{
		platformName = "gitlab";
		break;
	}
	}

	return fmt::format("https://{}.com/{}/{}", platformName, user, repository);
}

///////////////////////////////////////
std::string DownloadLocation::getBranch() const
{
	if (branch.empty())
		return {};

	if (exactBranch)
		return branch;
	
	return "pacc-" + branch;
}

///////////////////////////////////////
PackageVersions parseTagsToGetVersions(std::string const& lsRemoteOutput_)
{
	PackageVersions result;

	std::size_t numLines = std::count(lsRemoteOutput_.begin(), lsRemoteOutput_.end(), '\n') + 1;
	result.confirmed.reserve(numLines / 2);
	result.rest.reserve(numLines / 2);
	
	auto tryParseVersion = [](Version & ver, std::string const& str)
		{
			try {
				ver = Version::fromString(str);
			} catch(...) {
				return false;
			}
			return true;
		};

	for(auto token : StringTokenIterator(lsRemoteOutput_, "\r\n"))
	{
		if (token.empty())
			continue;

		std::size_t lastSlash = token.find_last_of("/");
		if (lastSlash == std::string_view::npos)
			continue;

		std::string tagName(token.substr(lastSlash + 1));

		Version ver;
		if (startsWith(tagName, "pacc-"))
		{
			if (!tryParseVersion(ver, tagName.substr(5)))
				continue;

			result.confirmed.push_back( { std::move(tagName), std::move(ver) } );
		}
		else if (startsWith(tagName, "v"))
		{
			if (!tryParseVersion(ver, tagName.substr(1)))
				continue;

			result.rest.push_back( { std::move(tagName), std::move(ver) } );
		}
		else
		{
			if (!tryParseVersion(ver, tagName))
				continue;
			
			result.rest.push_back( { std::move(tagName), std::move(ver) } );
		}
	}

	return result;
}
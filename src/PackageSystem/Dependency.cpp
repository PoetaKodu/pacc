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
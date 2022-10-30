#include "include/Pacc/PaccPCH.hpp"

#include <Pacc/PackageSystem/Dependency.hpp>

/////////////////////////////////////////////////
DownloadLocation DownloadLocation::parse(String const& depTemplate_)
{
	if (depTemplate_.empty())
		return {};

	DownloadLocation result;

	std::size_t colonPos = depTemplate_.find(':');
	String rest;
	String platformName;

	if (colonPos != String::npos)
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

	String repo;
	if (result.platform == OfficialRepo)
	{
		repo = std::move(rest);
	}
	else
	{
		std::size_t slashPos = rest.find('/');
		if (slashPos == String::npos)
		{
			throw PaccException("Invalid package \"{}\". Unknown user name.", depTemplate_)
				.withHelp("Use following syntax: \"{}:UserName/RepoName\"", platformName);
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
String DownloadLocation::getGitLink() const
{
	if (platform == DownloadLocation::Unknown)
		return "";

	String platformName;
	String user = userName;
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
String DownloadLocation::getBranch() const
{
	if (branch.empty())
		return {};

	if (exactBranch)
		return branch;

	return "pacc-" + branch;
}

///////////////////////////////////////
PackageVersions& PackageVersions::sort()
{
	rg::sort(confirmed, rg::greater{}, &StringVersionPair::second);
	rg::sort(rest, rg::greater{}, &StringVersionPair::second);

	return *this;
}

///////////////////////////////////////
PackageVersions PackageVersions::filter(VersionReq const& req_)
{
	Vec<StringVersionPair> c, r;
	c.reserve(confirmed.size());
	r.reserve(rest.size());

	auto meetsReq = [&](auto const& elem)
		{
			return req_.test(elem.second);
		};

	rg::copy_if(confirmed, std::back_inserter(c), meetsReq);
	rg::copy_if(rest, std::back_inserter(r), meetsReq);

	return PackageVersions{ std::move(c), std::move(r) };
}


///////////////////////////////////////
PackageVersions PackageVersions::parse(String const& lsRemoteOutput_)
{
	PackageVersions result;

	auto numLines = rg::count(lsRemoteOutput_, '\n') + 1;
	result.confirmed.reserve(numLines / 2);
	result.rest.reserve(numLines / 2);

	auto tryParseVersion = [](Version & ver, String const& str)
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

		auto lastSlash = token.find_last_of("/");
		if (lastSlash == StringView::npos)
			continue;

		auto tagName = String(token.substr(lastSlash + 1));

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

#include "git.hpp"
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <array>
#include <algorithm>
#include <sstream>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define pclose _pclose
#define popen _popen
#endif

namespace Git
{
	std::string Bridge::rtrim(std::string s)
	{
		s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
			return !std::isspace(ch);
			}).base(), s.end());
		return s;
	}

	std::string Bridge::CommitDescription::getAuthor() const
	{
		return authorName;
	}

	std::string Bridge::CommitDescription::getAuthorDate() const
	{
		return authorDate;
	}

	std::string Bridge::CommitDescription::getAuthorDateMessage() const
	{
		return authorDateMessage;
	}

	std::string Bridge::CommitDescription::getCommiter() const
	{
		return commiterName;
	}

	std::string Bridge::CommitDescription::getCommiterDate() const
	{
		return commiterDate;
	}

	std::string Bridge::CommitDescription::getCommiterDateMessage() const
	{
		return commiterDateMessage;
	}

	std::string Bridge::CommitDescription::getHash() const
	{
		return hash;
	}

	void Bridge::getBranches(std::vector<Branch>& ret, bool local)
	{
		std::string prefix = "branch";
		std::string suffix = " --format=%(refname:short)";
		std::string mid = local?"":" -r";

		std::string res = call(prefix+mid+suffix);

		std::istringstream iss(res);
		std::string item;
		while (std::getline(iss, item, '\n'))
		{
			size_t pos = item.find('/');
			if (pos != std::string::npos)
			{
				std::string remote = item.substr(pos + 1);
				if (remote != "HEAD")
					ret.emplace_back(remote, item.substr(0, pos));
			}
			else
			{
				ret.emplace_back(item,"");
			}
		}
	}
	std::string Bridge::call(const std::string& args)
	{
		std::string cmd = gitCall;
		if (path.length() > 0)
			cmd = std::string("cd ") + path + " && " + gitCall;

		std::array<char, 128> buffer;
		std::string result;
		std::unique_ptr<FILE, decltype(&pclose)> pipe(popen((cmd+" "+args).c_str(), "r"), pclose);
		if (!pipe) {
			throw std::runtime_error("popen() failed!");
		}
		while (fgets(buffer.data(), static_cast<unsigned>(buffer.size()), pipe.get()) != nullptr) {
			result += buffer.data();
		}
		return result;
	}
	Bridge::Branch Bridge::getCurrentBranch()
	{
		return Branch(rtrim(call("rev-parse --abbrev-ref HEAD")));
	}
	std::vector<Bridge::Branch> Bridge::getBranches(bool remote, bool local)
	{
		std::vector<Branch> res;
		if (local)
			getBranches(res, true);
		if (remote)
			getBranches(res, false);
		return res;
	}
	std::vector<Bridge::CommitDescription> Bridge::getCommitDescriptions(size_t count, size_t start)
	{
		std::vector<CommitDescription> ret;

		std::string args = "log ";
		if (count > 0)
			args += "--max-count=" + std::to_string(count) + " ";
		if (start > 0)
			args += "--skip=" + std::to_string(start) + " ";

		std::string res = call(args+" --pretty=format:\"%H|%aN<%ae>|%ar|%aI|%cN<%ce>|%cr|%cI|%B\"");

		size_t pos = res.find("\n"),lpos = 0;

		while (pos != res.npos && pos < res.size())
		{
			std::string v = res.substr(lpos, pos - lpos);

			size_t subpos = v.find("|"), sublpos = 0;

			std::string hash = v.substr(0,subpos);

			sublpos = subpos+1;
			subpos = v.find("|", subpos+1);
			std::string authorName = v.substr(sublpos, subpos - sublpos);

			sublpos = subpos+1;
			subpos = v.find("|", subpos+1);
			std::string authorDateMessage = v.substr(sublpos, subpos - sublpos);

			sublpos = subpos+1;
			subpos = v.find("|", subpos+1);
			std::string authorDate = v.substr(sublpos, subpos - sublpos);

			sublpos = subpos+1;
			subpos = v.find("|", subpos+1);
			std::string commiterName = v.substr(sublpos, subpos - sublpos);

			sublpos = subpos+1;
			subpos = v.find("|", subpos+1);
			std::string commiterDateMessage = v.substr(sublpos, subpos - sublpos);

			sublpos = subpos+1;
			subpos = v.find("|", subpos+1);
			std::string commiterDate = v.substr(sublpos, subpos - sublpos);

			std::string message = v.substr(subpos+1);

			ret.emplace_back(hash,authorName,authorDate,authorDateMessage,commiterName,commiterDate,commiterDateMessage);

			lpos = pos+2;
			pos = res.find("\n", pos+2);
		}

		return ret;
	}
	Bridge::CommitStatus Bridge::getCommitStatus(const CommitDescription& commit)
	{
		CommitStatus status;

		commit.getHash();

		//std::cout << call("git status ");

		return CommitStatus();
	}
}
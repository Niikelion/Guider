#pragma once

#include <string>
#include <vector>
#include <iostream>

namespace Git
{
	class Bridge
	{
	public:
		class Branch;
	private:

		std::string gitCall;
		std::string path;
		bool isInWorkingTree;

		std::string rtrim(std::string s);

		void getBranches(std::vector<Branch>& ret, bool local);
	public:
		class Branch
		{
		private:
			std::string name;
			std::string origin;
		public:
			inline std::string getName() const
			{
				return name;
			}
			inline std::string getOrigin() const
			{
				return origin;
			}
			inline bool isLocal() const
			{
				return origin.length() == 0;
			}
			Branch(const std::string& branch,const std::string& remote="") : name(branch),origin(remote) {}
			Branch(const Branch& branch) = default;
		};
		class CommitDescription
		{
		private:
			std::string hash;
			
			std::string authorName;
			std::string authorDate;
			std::string authorDateMessage;
			
			std::string commiterName;
			std::string commiterDate;
			std::string commiterDateMessage;
		public:
			std::string getAuthor() const;
			std::string getAuthorDate() const;
			std::string getAuthorDateMessage() const;
			std::string getCommiter() const;
			std::string getCommiterDate() const;
			std::string getCommiterDateMessage() const;
			std::string getHash() const;

			CommitDescription(const std::string& h, const std::string& an,const std::string& ad, const std::string& adm, const std::string& cn, const std::string& cd, const std::string& cdm) : hash(h), authorName(an), authorDate(ad), authorDateMessage(adm), commiterName(cn), commiterDate(cd), commiterDateMessage(cdm) {};
			CommitDescription(const CommitDescription&) = default;
		};
		class LineChanges
		{
		public:
			enum Status
			{
				Unmodified,
				Inserted,
				Deleted
			};
			Status status;
			std::string localLine, sourceLine;
		};
		class FileChanges
		{
		public:
			std::vector<LineChanges>  lines;
		};
		class CommitChanges
		{
		public:
			std::vector<FileChanges> files;
		};
		class FileStatus
		{
		public:
			std::string filename;
			enum Status
			{
				Unmodified,
				Modified,
				Added,
				Deleted,
				Renamed,
				Copied,
				Updated
			};
			Status status;
		};
		class CommitStatus
		{
		public:
			std::vector<FileStatus> files;
		};
		
		inline void setPath(const std::string& directory)
		{
			path = directory;
			std::string result = call("rev-parse --is-inside-work-tree 2>&1");
			isInWorkingTree = (rtrim(result) == "true");
		}
		inline bool isInRepository()
		{
			return isInWorkingTree;
		}

		std::string call(const std::string& args);
		inline std::string call(const std::vector<std::string>& args)
		{
			std::string re;
			for (const auto& arg : args)
				re += " " + arg;
			return call(re);
		}

		std::string getCurrentBranch();

		std::vector<Branch> getBranches(bool remote = true, bool local = true);
		std::vector<CommitDescription> getCommitDescriptions(size_t count = 0,size_t start = 0);

		CommitStatus getCommitStatus(const CommitDescription& commit);

		Bridge(const std::string& directory = "",const std::string& callCommand = "git"): gitCall(callCommand), path(""), isInWorkingTree(false)
		{
			setPath(directory);
		};
	};
}
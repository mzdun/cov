// Copyright (c) 2022 Marcin Zdun
// This code is licensed under MIT license (see LICENSE for details)

#pragma once
#include <cov/db.hh>
#include <cov/discover.hh>
#include <cov/error.hh>
#include <cov/git2/config.hh>
#include <cov/git2/odb.hh>
#include <cov/git2/oid.hh>
#include <cov/git2/repository.hh>
#include <cov/init.hh>
#include <cov/reference.hh>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace git {
	struct blob;
}  // namespace git

namespace cov {
	enum class origin {
		git,
		cov,
		neither,
	};

	struct blob : object {
		obj_type type() const noexcept override { return obj_blob; }
		bool is_blob() const noexcept final { return true; }
		virtual git::blob const& peek() const noexcept = 0;
		virtual git::blob peel_and_unlink() noexcept = 0;
		virtual cov::origin origin() const noexcept = 0;

		static ref_ptr<blob> wrap(git::blob&& ref, cov::origin);
	};

	template <typename Int = unsigned>
	struct multi_ratio {
		io::v1::stats::ratio<Int> lines{};
		io::v1::stats::ratio<Int> functions{};
		io::v1::stats::ratio<Int> branches{};

		static multi_ratio<Int> calc(io::v1::coverage_stats const& stats,
		                             unsigned char digits) {
			return {
			    .lines = stats.lines.calc(digits),
			    .functions = stats.functions.calc(digits),
			    .branches = stats.branches.calc(digits),
			};
		}

		static multi_ratio<int> diff(multi_ratio<> const& newer,
		                             multi_ratio<> const& older) {
			return {
			    .lines = io::v1::diff(newer.lines, older.lines),
			    .functions = io::v1::diff(newer.functions, older.functions),
			    .branches = io::v1::diff(newer.branches, older.branches),
			};
		}
	};

	struct file_diff {
		enum kind { normal, renamed, copied, added, deleted };
		enum initial { initial_add_all, initial_with_self };

		struct {
			multi_ratio<> current{};
			multi_ratio<int> diff{};
		} coverage;
		struct {
			io::v1::coverage_stats current{};
			io::v1::coverage_diff diff{};
		} stats;
	};

	struct file_stats {
		std::string filename{};
		io::v1::coverage_stats current{};
		io::v1::coverage_stats previous{};
		std::string previous_name{};
		file_diff::kind diff_kind{};
		git::oid current_functions{};
		git::oid previous_functions{};

		bool operator==(file_stats const&) const noexcept = default;

		file_diff diff(unsigned char digits = 2) const noexcept {
			auto const curr = multi_ratio<>::calc(current, digits);
			auto const prev = multi_ratio<>::calc(previous, digits);
			return {
			    .coverage = {.current = curr,
			                 .diff = multi_ratio<>::diff(curr, prev)},
			    .stats = {.current = current,
			              .diff = io::v1::diff(current, previous)},
			};
		}
	};

	struct commit_file_diff {
		std::string previous_name{};
		file_diff::kind diff_kind{};
	};

	struct current_head_type {
		std::string branch{};
		std::optional<git::oid> tip{};
		ref_ptr<cov::reference> ref{};

		bool operator==(current_head_type const& other) const noexcept {
			return branch == other.branch &&
			       tip.has_value() == other.tip.has_value() &&
			       (!tip.has_value() || *tip == *other.tip);
		}
	};

	struct repository {
		repository();
		repository(repository&&);
		repository& operator=(repository&&);
		~repository();

		static std::filesystem::path discover(
		    std::filesystem::path const& current_dir,
		    discover across_fs,
		    std::error_code& ec) {
			return discover_repository(current_dir, across_fs, ec);
		}

		static std::filesystem::path discover(
		    std::filesystem::path const& current_dir,
		    std::error_code& ec) {
			return discover(current_dir, discover::within_fs, ec);
		}

		static repository init(std::filesystem::path const& sysroot,
		                       std::filesystem::path const& cov_dir,
		                       std::filesystem::path const& git_dir,
		                       std::error_code& ec,
		                       init_options const& options = {}) {
			auto cov_directory = init_repository(cov_dir, git_dir, ec, options);
			if (ec) return repository{};
			return repository{sysroot, cov_directory, ec};
		}

		static repository open(std::filesystem::path const& sysroot,
		                       std::filesystem::path const& cov_dir,
		                       std::error_code& ec) {
			return repository{sysroot, cov_dir, ec};
		}

		std::filesystem::path const& cov_dir() const noexcept {
			return cov_dir_;
		}

		std::filesystem::path const& common_dir() const noexcept {
			return common_dir_;
		}

		std::string_view git_dir() const noexcept {
			return git_.repo().git_dir();
		}

		std::optional<std::string_view> git_work_dir() const noexcept {
			return git_.repo().work_dir();
		}

		std::string_view git_common_dir() const noexcept {
			return git_.repo().common_dir();
		}

		git::repository_handle git() const noexcept { return git_.repo(); }

		git::config const& config() const noexcept { return cfg_; }
		ref_ptr<references> const& refs() const noexcept { return refs_; }
		current_head_type current_head() const;
		bool update_current_head(git::oid_view ref,
		                         current_head_type const& known) const;
		ref_ptr<object> dwim(std::string_view) const;
		ref_ptr<object> find_partial(std::string_view partial) const;
		ref_ptr<object> find_partial(git::oid_view in,
		                             size_t character_count) const;
		template <typename Object>
		ref_ptr<Object> lookup(git::oid_view id, std::error_code& ec) const {
			auto object = lookup_object(id, ec);
			return as_a<Object>(std::move(object), ec);
		}
		bool write(git::oid&, ref_ptr<object> const&);
		bool write(git::oid& out, git::bytes const& bytes) {
			return git_.write(out, bytes);
		}

		std::map<std::string, commit_file_diff> diff_betwen_commits(
		    git::oid_view newer,
		    git::oid_view older,
		    std::error_code& ec,
		    git_diff_find_options const* opts = nullptr) const;

		std::vector<file_stats> diff_betwen_reports(
		    ref_ptr<report> const& newer,
		    ref_ptr<report> const& older,
		    std::error_code& ec,
		    git_diff_find_options const* opts = nullptr) const;

		std::vector<file_stats> diff_with_parent(
		    ref_ptr<report> const& current,
		    std::error_code& ec,
		    git_diff_find_options const* opts = nullptr,
		    file_diff::initial initial_policy =
		        file_diff::initial_with_self) const;

	protected:
		explicit repository(std::filesystem::path const& sysroot,
		                    std::filesystem::path const& cov_dir,
		                    std::error_code&);

		ref_ptr<object> lookup_object(git::oid_view id, std::error_code&) const;

	private:
		struct git_repo {
			git_repo();
			void open(std::filesystem::path const& common_dir,
			          std::filesystem::path const& cov_dir,
			          git::config const& cfg,
			          std::error_code&);
			ref_ptr<blob> lookup(git::oid_view id, std::error_code&) const;
			bool write(git::oid&, git::bytes const&);

			git::repository_handle repo() const noexcept { return git_; }

		private:
			git::repository git_{};
			git::repository local_{};
			git::odb odb_{};
		};

		std::filesystem::path cov_dir_{};
		std::filesystem::path common_dir_{};
		git::config cfg_{};
		git_repo git_{};
		ref_ptr<references> refs_{};
		ref_ptr<backend> db_{};
	};
}  // namespace cov

// Copyright (c) 2022 Marcin Zdun
// This code is licensed under MIT license (see LICENSE for details)

#include <cov/discover.hh>
#include <cov/error.hh>
#include <cov/git2/repository.hh>
#include <cov/init.hh>
#include <cov/io/file.hh>
#include "path-utils.hh"

#ifdef WIN32
#include <Windows.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

namespace cov {
	namespace {
		std::optional<path> read_covlink(path const& basename) {
			std::error_code ec{};
			if (!is_regular_file(basename, ec) || ec) return std::nullopt;
			auto in = io::fopen(basename);
			auto line = in.read_line();
			auto view = prefixed(names::covlink_prefix, line);
			if (!view) return std::nullopt;
			return weakly_canonical(basename.parent_path() / make_path(*view));
		}

		auto device_id(path const& name) {
#ifdef WIN32
#define STAT _wstat
			using stat_type = struct _stat;
#else
#define STAT stat
			using stat_type = struct stat;
#endif

			stat_type st{};
			using result_type = std::optional<decltype(st.st_dev)>;
			if (STAT(name.c_str(), &st)) return result_type{};
			return result_type{st.st_dev};
		}
	}  // namespace

#define TRY_EX(DIRNAME, RET)                \
	do {                                    \
		auto local = (DIRNAME);             \
		if (is_valid_path(local)) RET;      \
		auto covlink = read_covlink(local); \
		if (covlink) {                      \
			local = std::move(*covlink);    \
			if (is_valid_path(local)) RET;  \
		}                                   \
	} while (0)

#define TRY(DIRNAME) TRY_EX(DIRNAME, return weakly_canonical(local, ec))

	std::filesystem::path discover_repository(
	    std::filesystem::path const& current_dir,
	    discover across_fs,
	    std::error_code& ec) {
		if (is_valid_path(current_dir))
			return weakly_canonical(current_dir, ec);

		TRY(current_dir / names::covdata_dir);
		if (auto const git_dir = git::repository::discover(
		        current_dir,
		        across_fs == discover::across_fs ? git::Discover::AcrossFs
		                                         : git::Discover::WithinFs,
		        ec);
		    !git_dir.empty()) {
			TRY(git_dir / names::covdata_dir);
			std::error_code local_ec{};
			auto const repo = git::repository::open(git_dir, local_ec);
			if (!local_ec) {
				auto const common_dir = repo.common_dir();
				if (!common_dir.empty()) {
					TRY_EX(make_path(common_dir) / names::covdata_dir, {
						ec = make_error_code(errc::uninitialized_worktree);
						return {};
					});
				}
			}
		}

		auto dirname = absolute(current_dir);
		decltype(device_id(dirname)) device = std::nullopt;

		while (true) {
			{
				auto parent = dirname.parent_path();
				if (parent == dirname) break;
				dirname = std::move(parent);
				if (across_fs == discover::within_fs) {
					auto current_device = device_id(dirname);
					if (current_device) {
						if (!device) {
							device = current_device;
						} else if (*device != *current_device) {
							return {};  // GCOV_EXCL_LINE -- untestable
						}               // GCOV_EXCL_LINE
					}
				}
			}
			if (is_valid_path(dirname)) return weakly_canonical(dirname, ec);
			TRY(dirname / names::covdata_dir);
		}

		ec = make_error_code(git::errc::notfound);
		return {};
	}

#undef TRY
}  // namespace cov

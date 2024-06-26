// Copyright (c) 2022 Marcin Zdun
// This code is licensed under MIT license (see LICENSE for details)

#include <cov/app/args.hh>
#include <cov/app/dirs.hh>
#include <cov/app/path.hh>
#include <cov/app/rt_path.hh>
#include <cov/repository.hh>

namespace cov::app {
	cov::repository open_here(parser_holder const& parser,
	                          str::errors::Strings const& err_str,
	                          str::args::Strings const& arg_str) {
		std::error_code ec{};
		auto const repo_path = parser.locate_repo_here(err_str, arg_str);
		auto repo = cov::repository::open(platform::sys_root(), repo_path, ec);
		if (ec) parser.error(ec, err_str, arg_str);

		return repo;
	}
}  // namespace cov::app

namespace cov::app::platform {
	std::filesystem::path const& sys_root() {
		// dirname / ..
		static auto const root = exec_path().parent_path().parent_path();
		return root;
	}

	std::filesystem::path locale_dir() {
		return sys_root() / directory_info::share / "locale"sv;
	}  // GCOV_EXCL_LINE
}  // namespace cov::app::platform

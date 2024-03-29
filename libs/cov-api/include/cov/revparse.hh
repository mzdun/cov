// Copyright (c) 2022 Marcin Zdun
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <cov/git2/oid.hh>

namespace cov {
	struct repository;

	struct revs {
		git::oid from{};
		git::oid to{};
		bool single{false};

		static std::error_code parse(cov::repository const& repo,
		                             std::string_view range,
		                             revs& out);

		static std::error_code parse_single(cov::repository const& repo,
		                                    std::string_view rev,
		                                    git::oid& out);
		static bool is_report(cov::repository const& repo, git::oid_view id);

	private:
		void locate_range(cov::repository const& repo);
	};
}  // namespace cov

// Copyright (c) 2024 Marcin Zdun
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <compare>
#include <set>
#include <span>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

namespace cov::app::strip {
	struct excl_block {
		unsigned start{};
		unsigned end{};  // GCOV_EXCL_LINE
		auto operator<=>(excl_block const&) const noexcept = default;
	};

	using exclude_result =
	    std::pair<std::vector<excl_block>, std::set<unsigned>>;

	struct exclude_report_line {
		unsigned line;
		std::string counter;  // GCOV_EXCL_LINE
		std::string text;     // GCOV_EXCL_LINE
		auto operator<=>(exclude_report_line const&) const noexcept = default;
	};

	struct exclude_report {
		std::string path{};
		std::vector<exclude_report_line> lines{};
	};

	struct excludes {
		std::string path;
		std::span<std::string_view const> valid_markers;
		bool inside_exclude{false};
		std::tuple<unsigned, unsigned, std::string> last_start{};
		std::vector<unsigned> single_lines{};
		std::vector<excl_block> result{};
		std::set<unsigned> empties{};

		bool has_any_marker(std::string_view markers);
		void on_line(unsigned line_no, std::string_view text);
		void after_lines();
		void exclude(unsigned line);
		void include_back(unsigned line);
		void warn(unsigned line, unsigned column, std::string_view msg);
		void note(unsigned line, unsigned column, std::string_view msg);
		void message(unsigned line,
		             unsigned column,
		             std::string_view tag,
		             std::string_view msg);
		static std::string stop_for(std::string_view start,
		                            std::string_view suffix);
	};

}  // namespace cov::app::strip

// Copyright (c) 2023 Marcin Zdun
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <cov/io/types.hh>
#include <hilite/lighter.hh>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace cov::app {
	struct view_columns {
		size_t line_no_width;
		size_t count_width;
	};

	struct cvg_info {
		std::map<unsigned, unsigned> coverage{};
		std::vector<std::pair<unsigned, unsigned>> chunks{};
		std::string_view file_text{};
		lighter::highlights syntax{};

		static cvg_info from_coverage(
		    std::vector<io::v1::coverage> const& lines);
		void load_syntax(std::string_view text, std::string_view filename);
		view_columns column_widths() const noexcept;

		std::optional<unsigned> max_count() const noexcept;
		std::optional<unsigned> count_for(unsigned line_no) const noexcept;

		bool has_line(size_t line_no) const noexcept {
			return syntax.lines.size() > line_no;
		}
		std::string to_string(size_t line_no,
		                      view_columns const& widths,
		                      bool use_color) const noexcept;
	};
}  // namespace cov::app

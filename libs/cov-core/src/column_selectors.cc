// Copyright (c) 2024 Marcin Zdun
// This code is licensed under MIT license (see LICENSE for details)

#include "column_selectors.hh"

namespace cov::core {
	namespace {
		template <typename Enum>
		static Enum mark_color(translatable mark,
		                       Enum pass,
		                       Enum incomplete,
		                       Enum fail) {
			return mark == cov::translatable::mark_failing      ? fail
			       : mark == cov::translatable::mark_incomplete ? incomplete
			                                                    : pass;
		}

		static auto const& select_rating(placeholder::rating const& marks,
		                                 placeholder::stats fld) {
			using enum placeholder::stats;
			return fld == lines_rating       ? marks.lines
			       : fld == functions_rating ? marks.functions
			                                 : marks.branches;
		}

		io::v1::stats::ratio<> zero_to_one_hundred(
		    io::v1::stats::ratio<> const& value,
		    cov::io::v1::stats const& base) {
			if (!base.relevant) return {100u, 0u, value.digits};
			return value;
		}

		io::v1::stats::ratio<int> zero_to_one_hundred(
		    io::v1::stats::ratio<int> const& value,
		    cov::io::v1::stats const& base) {
			if (!base.relevant) return {0, 0, value.digits};
			return value;
		}

		cov::io::v1::stats zero_to_one_hundred(
		    cov::io::v1::stats const& value) {
			if (!value.relevant) return {1u, 1u};
			return value;
		}
	}  // namespace

	cell_info ratio_selector::cell(placeholder::rating const& marks,
	                               file_diff const& diff) const {
		auto const original_stats = this->stats(diff.stats.current);
		auto const stats = zero_to_one_hundred(original_stats);
		auto const value = zero_to_one_hundred(
		    this->current(diff.coverage.current), original_stats);
		auto const change =
		    zero_to_one_hundred(this->diff(diff.coverage.diff), original_stats);
		auto const mark = formatter::apply_mark(
		    stats, select_rating(marks, this->rating_selector()));
		auto const category =
		    mark_color(mark, value_category::passing,
		               value_category::incomplete, value_category::failing);

		return {
		    .value = format_value(value),
		    .change = format_signed_value(change),
		    .category = category,
		    .change_is_negative = is_neg(change),
		};
	}

	cell_info counter_selector::cell(placeholder::rating const&,
	                                 file_diff const& diff) const {
		auto const value = this->current(diff.stats.current);
		auto const change = this->diff(diff.stats.diff);
		auto change_is_negative = is_neg(change);
		if (this->change_direction() == change_is::reversed)
			change_is_negative = !change_is_negative;

		return {
		    .value = format_value(value, this->is_dimmed()),
		    .change = format_signed_value(change),
		    .category = value_category::passing,
		    .change_is_negative = change_is_negative,
		};
	}
}  // namespace cov::core

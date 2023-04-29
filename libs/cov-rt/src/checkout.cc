// Copyright (c) 2023 Marcin Zdun
// This code is licensed under MIT license (see LICENSE for details)

#include <fmt/format.h>
#include <cov/app/checkout.hh>
#include <cov/app/path.hh>
#include <cov/app/rt_path.hh>
#include <cov/format.hh>
#include <cov/repository.hh>
#include <cov/revparse.hh>

namespace fs = std::filesystem;
using namespace std::literals;

namespace cov::app::checkout {
	namespace {
		using str::args::lng;

		struct cmd_info {
			std::pair<unsigned, unsigned> minmax;
			parser::args_description descr;
		};

		static constexpr cmd_info items[] = {
		    {{1, 1},
		     {lng::NAME_META,
		      "checks out a branch, if it exists; otherwise, checks out and detaches from a commit or a tag"sv}},
		    {{0, 1},
		     {"--detach"sv,
		      "detaches the HEAD, checking out if the name is provided"sv,
		      opt{lng::NAME_META}}},
		    {{1, 2},
		     {"-b"sv,
		      "for non-existing branches, creates a branch and checks it out"sv,
		      lng::NAME_META, opt{"<start-point>"sv}}},
		    {{1, 1},
		     {"--orphan"sv, "creates and checks out an orphan branch"sv,
		      lng::NAME_META}},
		};

		template <auto index, size_t size>
		struct is_within {
			inline static constexpr cmd_info const& get() {
				static_assert(index > 0, "Index must start from 1.");
				static_assert(index < (size + 1),
				              "Items and commands got out of sync.");
				return items[index - 1];
			}
		};

		template <command Cmd>
		inline constexpr cmd_info const& info() {
			return is_within<std::to_underlying(Cmd), std::size(items)>::get();
		}

		template <command Cmd>
		inline constexpr parser::string name_of() noexcept {
			return info<Cmd>().descr.args;
		}
		inline constexpr parser::string name_of(command cmd) noexcept {
			auto const index = std::to_underlying(cmd) - 1;
			return items[index].descr.args;
		}
		inline constexpr std::pair<unsigned, unsigned> minmax(
		    command cmd) noexcept {
			auto const index = std::to_underlying(cmd) - 1;
			return items[index].minmax;
		}

		[[noreturn]] void show_help(::args::parser& p) {
			using enum command;

			auto prn = args::printer{stdout};
			auto& _ = parser::base::tr(p);

			auto const _s = [&](auto arg) { return to_string(_(arg)); };

			auto name_meta = _(lng::NAME_META);
			auto commit_meta = "<commit>"sv;
			auto start_point_meta = "<start-point>"sv;
			prn.format_paragraph(_s(lng::USAGE), 7);
			for (auto synopsis : {
			         " cov checkout [-h] {0}"sv,
			         " cov checkout [-h] --detach [{0}]"sv,
			         " cov checkout [-h] [--detach] {1}"sv,
			         " cov checkout [-h] [-b|-B] {0} [{2}]"sv,
			         " cov checkout [-h] --orphan {0}"sv,
			     }) {
				prn.format_paragraph(
				    fmt::format(fmt::runtime(synopsis), name_meta, commit_meta,
				                start_point_meta),
				    7);
			}

			static const parser::args_description positionals[] = {
			    info<checkout>().descr,
			};

			static const parser::args_description optionals[] = {
			    {"-h, --help"sv, lng::HELP_DESCRIPTION},
			    info<detach>().descr,
			    info<branch_and_checkout>().descr,
			    info<checkout_orphaned>().descr,
			};

			args::fmt_list args(2);
			parser::args_description::group(_, lng::POSITIONALS, positionals,
			                                args[0]);
			parser::args_description::group(_, lng::OPTIONALS, optionals,
			                                args[1]);

			prn.format_list(args);
			std::exit(0);
		}  // GCOV_EXCL_LINE

		template <typename Reference>
		std::error_code set_head(parser const& p,
		                         repository const& repo,
		                         Reference const& full_name_or_oid) {
			if (!repo.refs()->create("HEAD"sv, full_name_or_oid))
				p.error("cannot write to HEAD");
			return {};
		}

		std::error_code detach_ref(parser const& p,
		                           repository const& repo,
		                           std::string_view name) {
			git_oid oid{};
			auto const ec = revs::parse_single(repo, name, oid);
			if (ec) return ec;

			return set_head(p, repo, oid);
		}

		std::error_code checkout_ref(parser const& p,
		                             repository const& repo,
		                             std::string_view name) {
			auto const ref = repo.refs()->dwim(name);
			if (ref && ref->references_branch()) {
				return set_head(p, repo, ref->name());
			}

			return detach_ref(p, repo, name);
		}

		std::error_code branch_checkout_ref(parser const& p,
		                                    repository const& repo,
		                                    std::string_view name) {
			auto const HEAD = repo.current_head();
			if (!HEAD.ref) return git::make_error_code(git::errc::error);

			std::error_code ec{};
			auto const branch =
			    repo.refs()->copy_ref(HEAD.ref, name, true, false, ec);
			if (ec && ec != git::errc::unbornbranch) return ec;

			return set_head(
			    p, repo,
			    branch ? branch->name() : fmt::format("refs/heads/{}", name));
		}

		std::error_code checkout_orphaned(parser const& p,
		                                  repository const& repo,
		                                  std::string_view name) {
			auto const full_name = fmt::format("refs/heads/{}", name);
			if (!reference::is_valid_name(full_name))
				return git::make_error_code(git::errc::invalidspec);
			if (repo.refs()->lookup(full_name))
				return git::make_error_code(git::errc::exists);

			return set_head(p, repo, full_name);
		}
	}  // namespace

	parser::parser(::args::args_view const& arguments,
	               str::translator_open_info const& langs)
	    : base{langs, arguments} {
		using enum command;
		parser_.usage(fmt::format("[-h] [--detach|-b|--orphan] {0} [{1}]"sv,
		                          tr_(lng::NAME_META), "<start-point>"sv));
		parser_.provide_help(false);
		parser_.custom(show_help, "h", "help").opt();
		parser_.custom(set_command<branch_and_checkout>(), "b").opt();
		parser_.custom(set_command<detach>(), "detach").opt();
		parser_.custom(set_command<checkout_orphaned>(), "orphan").opt();
		parser_.custom([this](std::string const& val) { handle_arg(val); })
		    .opt();
	}

	void parser::handle_command(command arg) {
		if (cmd_ != command::unspecified) {
			str_visitor visitor{tr_};
			error(tr_.format(
			    lng::ARGUMENT_MSG, visitor.visit(name_of(cmd_)),
			    tr_.format(lng::EXCLUSIVE, visitor.visit(name_of(arg)))));
		}

		cmd_ = arg;
	}

	void parser::handle_arg(std::string const& val) {
		if (cmd_ == command::unspecified) cmd_ = command::checkout;
		names_.push_back(val);
	}

	void parser::parse() {
		parser_.parse();
		if (cmd_ == command::unspecified) {
			error("at least one argument is needed");
		}

		str_visitor visitor{tr_};

		auto const [min, max] = minmax(cmd_);
		if (min == max) {
			if (names_.size() != max) {
				error(fmt::format(
				    fmt::runtime(tr_(modcnt::ERROR_OPTS_NEEDS_EXACTLY, max)),
				    visitor.visit(name_of(cmd_)), max));
			}
		}
		if (names_.size() > max) {
			error(fmt::format(
			    fmt::runtime(tr_(modcnt::ERROR_OPTS_NEEDS_AT_MOST, max)),
			    visitor.visit(name_of(cmd_)), max));
		}
		if (min && names_.size() < min) {
			error(fmt::format(
			    fmt::runtime(tr_(modcnt::ERROR_OPTS_NEEDS_AT_LEAST, min)),
			    visitor.visit(name_of(cmd_)), min));
		}
	}

	cov::repository parser::open_here() const {
		return cov::app::open_here(*this, tr_);
	}

	std::error_code parser::run(cov::repository const& repo) const {
		switch (cmd_) {
			case command::detach:
				return detach_ref(*this, repo,
				                  names_.empty() ? "HEAD"sv : names_.front());
			case command::checkout:
				return checkout_ref(*this, repo, names_.front());
			case command::branch_and_checkout:
				return branch_checkout_ref(*this, repo, names_.front());
			case command::checkout_orphaned:
				return checkout_orphaned(*this, repo, names_.front());

			default:    // GCOV_EXCL_LINE
				break;  // GCOV_EXCL_LINE
		}
		return {};  // GCOV_EXCL_LINE
	}
}  // namespace cov::app::checkout
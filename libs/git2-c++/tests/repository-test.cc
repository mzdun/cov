// Copyright (c) 2022 midnightBITS
// This code is licensed under MIT license (see LICENSE for details)

#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <git2-c++/commit.hh>
#include <git2-c++/repository.hh>
#include "setup.hh"

namespace git::testing {
	using namespace ::std::literals;
	using setup::get_path;
	using setup::make_path;
	using std::filesystem::path;
	using ::testing::TestWithParam;
	using ::testing::ValuesIn;

	path append(path const& root, std::string_view utf8) {
		return root / make_path(utf8);
	}

	path make_absolute(std::string_view utf8, bool absolute = false) {
		if (absolute) return make_path(utf8);

		return append(setup::test_dir(), utf8);
	}

	enum class repo_kind { failing, bare, workspace };
	struct repo_param {
		std::string_view start_path{};
		struct {
			std::string_view discovered{};
			std::string_view workdir{};
		} expected{};
		repo_kind kind{repo_kind::workspace};
	};

	class repository : public TestWithParam<repo_param> {};

	TEST_P(repository, discover) {
		auto [start_path, expected, kind] = GetParam();
		auto const start =
		    make_absolute(start_path, kind == repo_kind::failing);
		auto const result = git::repository::discover(start);
		if (expected.discovered == std::string_view{}) {
			ASSERT_EQ(get_path(result), std::string{});
			return;
		}

		ASSERT_EQ(get_path(result),
		          get_path(make_absolute(expected.discovered,
		                                 kind == repo_kind::failing)));
	}

	TEST_P(repository, open) {
		auto [start_path, expected, kind] = GetParam();
		auto const start =
		    make_absolute(start_path, kind == repo_kind::failing);
		auto const result = git::repository::discover(start);
		if (expected.discovered == std::string_view{}) {
			ASSERT_EQ(get_path(result), std::string{});
		} else {
			ASSERT_EQ(get_path(result),
			          get_path(make_absolute(expected.discovered,
			                                 kind == repo_kind::failing)));
		}

		if (!result.empty()) {
			auto const repo = git::repository::open(result);
			ASSERT_TRUE(repo);
		}
	}

	TEST_P(repository, wrap) {
		auto [start_path, expected, kind] = GetParam();
		auto const start =
		    make_absolute(start_path, kind == repo_kind::failing);
		auto const result = git::repository::discover(start);
		if (expected.discovered == std::string_view{}) {
			ASSERT_EQ(get_path(result), std::string{});
		} else {
			ASSERT_EQ(get_path(result),
			          get_path(make_absolute(expected.discovered,
			                                 kind == repo_kind::failing)));
		}

		if (!result.empty()) {
			auto const db = git::odb::open(result);
			auto const repo = git::repository::wrap(db);
			ASSERT_TRUE(db);
			ASSERT_TRUE(repo);
		}
	}

	TEST_P(repository, workdir) {
		auto [start_path, expected, kind] = GetParam();
		auto const start =
		    make_absolute(start_path, kind == repo_kind::failing);
		auto const result = git::repository::discover(start);
		if (expected.discovered == std::string_view{}) {
			ASSERT_EQ(get_path(result), std::string{});
		} else {
			ASSERT_EQ(get_path(result),
			          get_path(make_absolute(expected.discovered,
			                                 kind == repo_kind::failing)));
		}
		if (result.empty()) return;

		auto const repo = git::repository::open(result);
		ASSERT_TRUE(repo);
		auto const view = repo.workdir();
		if (kind == repo_kind::bare) {
			ASSERT_FALSE(view);
			return;
		}
		ASSERT_TRUE(view);
		ASSERT_EQ(*view, get_path(make_absolute(expected.workdir, false)));
	}

	TEST_P(repository, commondir) {
		auto [start_path, expected, kind] = GetParam();
		auto const start =
		    make_absolute(start_path, kind == repo_kind::failing);
		auto const result = git::repository::discover(start);
		if (expected.discovered == std::string_view{}) {
			ASSERT_EQ(get_path(result), std::string{});
		} else {
			ASSERT_EQ(get_path(result),
			          get_path(make_absolute(expected.discovered,
			                                 kind == repo_kind::failing)));
		}
		if (result.empty()) return;

		auto const repo = git::repository::open(result);
		ASSERT_TRUE(repo);
		auto const common = repo.commondir();
		ASSERT_EQ(get_path(result), common);
	}

	constexpr repo_param dirs[] = {
	    {"/"sv, {}, repo_kind::failing},
	    {"c:/"sv, {}, repo_kind::failing},
	    {"bare.git/"sv, {"bare.git/"sv}, repo_kind::bare},
	    {"bare.git"sv, {"bare.git/"sv}, repo_kind::bare},
	    {"gitdir/subdir/"sv, {"gitdir/.git/"sv, "gitdir/"sv}},
	    {"gitdir/"sv, {"gitdir/.git/"sv, "gitdir/"sv}},
	    {"gitdir"sv, {"gitdir/.git/"sv, "gitdir/"sv}},
	};

	INSTANTIATE_TEST_SUITE_P(dirs, repository, ValuesIn(dirs));

	TEST(repository, README) {
		auto const repo = git::repository::open(make_absolute("bare.git/"sv));
		ASSERT_TRUE(repo);
		auto const initial = repo.lookup<git::commit>(
		    "ed631389fc343f7788bf414c2b3e77749a15deb6"sv);
		ASSERT_TRUE(initial);
	}

}  // namespace git::testing
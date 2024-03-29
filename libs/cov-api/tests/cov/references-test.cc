// Copyright (c) 2022 Marcin Zdun
// This code is licensed under MIT license (see LICENSE for details)

#include <gtest/gtest.h>
#include <cov/reference.hh>
#include "path-utils.hh"
#include "setup.hh"

#ifdef WIN32
#include <winerror.h>
#endif

namespace cov::testing {
	using namespace std::literals;
	using ::testing::TestWithParam;
	using ::testing::ValuesIn;

	enum class REF { branch, tag, neither };

	struct references_test {
		std::string_view name;
		std::string_view ref_name;
		struct {
			std::string_view primary{};
			std::string_view secondary{};
			std::string_view name{};
			std::string_view shorthand{};
			REF primary_type{REF::neither};
			REF secondary_type{REF::neither};
			bool ref_exists{true};
		} expected{};
		std::vector<path_info> steps;

		friend std::ostream& operator<<(std::ostream& out,
		                                references_test const& param) {
			return out << param.name;
		}
	};

	class references : public TestWithParam<references_test> {};

	TEST_P(references, lookup) {
		auto const& [_, ref_name, expected, steps] = GetParam();

		{
			std::error_code ec{};
			path_info::op(steps, ec);
			ASSERT_FALSE(ec) << "   Error: " << ec.message() << " ("
			                 << ec.category().name() << ')';
		}

		auto refs =
		    cov::references::make_refs(setup::test_dir() / steps.front().name);
		ASSERT_TRUE(refs);
		ASSERT_EQ(obj_references, refs->type());
		ASSERT_TRUE(is_a<cov::references>(static_cast<object*>(refs.get())));
		ASSERT_FALSE(is_a<cov::reference>(static_cast<object*>(refs.get())));

		auto primary = refs->lookup(ref_name);

		if (!expected.ref_exists) {
			ASSERT_FALSE(primary);
			return;
		}

		ASSERT_TRUE(primary);
		ASSERT_EQ(obj_reference, primary->type());
		ASSERT_TRUE(is_a<cov::reference>(static_cast<object*>(primary.get())));
		ASSERT_FALSE(
		    is_a<cov::references>(static_cast<object*>(primary.get())));

		ASSERT_EQ(expected.primary_type == REF::branch,
		          primary->references_branch());
		ASSERT_EQ(expected.primary_type == REF::tag, primary->references_tag());

		if (primary->reference_type() == reference_type::direct) {
			auto const id = primary->direct_target();
			ASSERT_TRUE(id);
			ASSERT_EQ(expected.primary, id->str());
			ASSERT_TRUE(primary->symbolic_target().empty());
		} else {
			ASSERT_EQ(expected.primary, primary->symbolic_target());
			ASSERT_FALSE(primary->direct_target());
		}

		auto secondary = primary->peel_target();

		if (!expected.ref_exists) {
			ASSERT_FALSE(secondary);
			return;
		}

		ASSERT_TRUE(secondary);

		ASSERT_EQ(expected.secondary_type == REF::branch,
		          secondary->references_branch());
		ASSERT_EQ(expected.secondary_type == REF::tag,
		          secondary->references_tag());

		if (secondary->reference_type() == reference_type::direct) {
			auto const id = secondary->direct_target();
			ASSERT_TRUE(id);
			ASSERT_EQ(expected.secondary, id->str());
			ASSERT_TRUE(secondary->symbolic_target().empty());
		} else {
			ASSERT_EQ(expected.secondary, secondary->symbolic_target());
			ASSERT_FALSE(secondary->direct_target());
		}

		ASSERT_EQ(expected.name, secondary->name());
		ASSERT_EQ(expected.shorthand, secondary->shorthand());
	}

	static constexpr auto ID = "112233445566778899aabbccddeeff0012345678"sv;
	static constexpr auto ID_ = "112233445566778899aabbccddeeff0012345678\n"sv;

	static references_test const tests[] = {
	    {
	        "empty"sv,
	        {},
	        {.ref_exists = false},
	        make_setup(remove_all("empty"sv), create_directories("empty"sv)),
	    },
	    {
	        "missing"sv,
	        "refs/heads/missing"sv,
	        {.ref_exists = false},
	        make_setup(remove_all("missing"sv),
	                   create_directories("missing"sv)),
	    },
	    {
	        "HEAD-nonhex"sv,
	        "HEAD"sv,
	        {.ref_exists = false},
	        make_setup(remove_all("HEAD-nonhex"sv),
	                   create_directories("HEAD-nonhex"sv),
	                   touch("HEAD-nonhex/HEAD"sv,
	                         "112233445566778899aabbccddeeff00gg345678\n"sv)),
	    },
	    {
	        "HEAD-direct"sv,
	        "HEAD"sv,
	        {ID, ID, "HEAD"sv, "HEAD"sv},
	        make_setup(remove_all("HEAD-direct"sv),
	                   create_directories("HEAD-direct"sv),
	                   touch("HEAD-direct/HEAD"sv, ID_)),
	    },
	    {
	        "HEAD-main"sv,
	        "HEAD"sv,
	        {
	            "refs/heads/main"sv,
	            ID,
	            "refs/heads/main"sv,
	            "main"sv,
	            REF::neither,
	            REF::branch,
	        },
	        make_setup(remove_all("HEAD-main"sv),
	                   create_directories("HEAD-main"sv),
	                   touch("HEAD-main/refs/heads/main"sv, ID_),
	                   touch("HEAD-main/HEAD"sv, "ref: refs/heads/main\n")),
	    },
	    {
	        "ref-multi"sv,
	        "refs/tags/first"sv,
	        {
	            "refs/tags/intermediate"sv,
	            ID,
	            "refs/heads/main"sv,
	            "main"sv,
	            REF::tag,
	            REF::branch,
	        },
	        make_setup(remove_all("ref-multi"sv),
	                   create_directories("ref-multi"sv),
	                   touch("ref-multi/refs/heads/main"sv, ID_),
	                   touch("ref-multi/refs/tags/intermediate"sv,
	                         "ref: refs/heads/main\n"),
	                   touch("ref-multi/refs/tags/first"sv,
	                         "ref: refs/tags/intermediate\n")),
	    },
	    {
	        "ref-direct"sv,
	        "refs/tags/tag-name"sv,
	        {
	            ID,
	            ID,
	            "refs/tags/tag-name"sv,
	            "tag-name"sv,
	            REF::tag,
	            REF::tag,
	        },
	        make_setup(remove_all("ref-direct"sv),
	                   create_directories("ref-direct"sv),
	                   touch("ref-direct/refs/tags/tag-name"sv, ID_)),
	    },
	    {
	        "refs-direct"sv,
	        "refs/ref-name"sv,
	        {
	            ID,
	            ID,
	            "refs/ref-name"sv,
	            "ref-name"sv,
	            REF::neither,
	            REF::neither,
	        },
	        make_setup(remove_all("refs-direct"sv),
	                   create_directories("refs-direct"sv),
	                   touch("refs-direct/refs/ref-name"sv, ID_)),
	    },
	    {
	        "peel-invalid"sv,
	        "refs/tags/ref-name"sv,
	        {
	            "refs/in:out"sv,
	            {},
	            "refs/in:out"sv,
	            "in:out"sv,
	            REF::tag,
	            REF::neither,
	        },
	        make_setup(remove_all("refs-direct"sv),
	                   create_directories("refs-direct"sv),
	                   touch("refs-direct/refs/tags/ref-name"sv,
	                         "ref: refs/in:out\n")),
	    },
	};

	INSTANTIATE_TEST_SUITE_P(tests, references, ::testing::ValuesIn(tests));

	struct refnames_test {
		std::string_view name;
		bool valid{true};

		friend std::ostream& operator<<(std::ostream& out,
		                                refnames_test const& param) {
			return out << param.name << " (" << (param.valid ? "good" : "bad")
			           << ')';
		}
	};

	class refnames : public TestWithParam<refnames_test> {};

	TEST_P(refnames, validate) {
		auto const& [name, valid] = GetParam();

		ASSERT_EQ(valid, cov::reference::is_valid_name(name));
	}

	constexpr refnames_test good(std::string_view view) { return {view, true}; }
	constexpr refnames_test bad(std::string_view view) { return {view, false}; }

	static refnames_test const goods[] = {
	    good("HEAD"sv),
	    good("ORIG_HEAD"sv),
	    good("MAPPED"sv),
	    good("refs/name"sv),
	    good("refs/heads/main"sv),
	    good("refs/tags/v0.1.0-{-"sv),
	    good("refs/tags/v0.1.0-alpha"sv),
	    good("refs/tags/v0.1.0-alpha.lock.owner"sv),
	};

	INSTANTIATE_TEST_SUITE_P(good, refnames, ::testing::ValuesIn(goods));

	static refnames_test const bads[] = {
	    bad({}),
	    bad("HEAD0"sv),
	    bad("orig_head"sv),
	    bad("empty/"sv),
	    bad("refs/.name"sv),
	    bad("refs/before..after/main"sv),
	    bad("refs/tags/v0.1.0-@{-"sv),
	    bad("refs/tags/v0.1.0-alpha.lock"sv),
	    bad("refs/tags.lock/v0.1.0-alpha"sv),
	    bad("refs/in*out"sv),
	    bad("refs/in:out"sv),
	    bad("refs/in?out"sv),
	    bad("refs/in[out"sv),
	    bad("refs/in\"out"sv),
	    bad("refs/in^out"sv),
	    bad("refs/in~out"sv),
	    bad("refs/in out"sv),
	    bad("refs/in\tout"sv),
	    bad("refs/in:out"sv),
	};

	INSTANTIATE_TEST_SUITE_P(bad, refnames, ::testing::ValuesIn(bads));

	struct ctrl_refnames_test {
		char name;

		friend std::ostream& operator<<(std::ostream& out,
		                                ctrl_refnames_test const& param) {
			return out << static_cast<int>(param.name);
		}
	};

	class ctrl_refnames : public TestWithParam<ctrl_refnames_test> {};

	TEST_P(ctrl_refnames, validate) {
		auto const& [name] = GetParam();
		char buffer[] = "refs/in-out";
		buffer[7] = name;

		ASSERT_FALSE(
		    cov::reference::is_valid_name({buffer, std::size(buffer) - 1}));
	}

	static ctrl_refnames_test const ctrls[] = {
	    {'\x00'}, {'\x01'}, {'\x02'}, {'\x03'}, {'\x04'}, {'\x05'}, {'\x06'},
	    {'\x07'}, {'\x08'}, {'\x0a'}, {'\x0b'}, {'\x0c'}, {'\x0d'}, {'\x0e'},
	    {'\x0f'}, {'\x10'}, {'\x11'}, {'\x12'}, {'\x13'}, {'\x14'}, {'\x15'},
	    {'\x16'}, {'\x17'}, {'\x18'}, {'\x1a'}, {'\x1b'}, {'\x1c'}, {'\x1d'},
	    {'\x1e'}, {'\x1f'}, {'\x7f'},
	};

	INSTANTIATE_TEST_SUITE_P(ctrls, ctrl_refnames, ::testing::ValuesIn(ctrls));

	void ping(const char* id) { std::fprintf(stderr, ">>> %s\n", id); }

	bool onlyhex(std::string_view oid) {
		for (auto c : oid) {
			if (!std::isxdigit(static_cast<unsigned char>(c))) return false;
		}
		return true;
	}

	struct ref_info {
		std::string_view name;
		std::string_view ref;

		ref_ptr<reference> create_with(
		    ref_ptr<cov::references> const& peel_source) const {
			git_oid oid{};
			if (ref.size() == 40 && onlyhex(ref) &&
			    !git_oid_fromstr(&oid, ref.data())) {
				return reference::direct(cov::references::prefix_info(name),
				                         oid);
			}
			return reference::symbolic(cov::references::prefix_info(name),
			                           {ref.data(), ref.size()}, peel_source);
		}
	};

	struct remove_refs_test {
		std::string_view name;
		testing::ref_info ref_info;
		std::error_code expected;
		std::vector<path_info> steps;

		friend std::ostream& operator<<(std::ostream& out,
		                                remove_refs_test const& param) {
			return out << param.name;
		}
	};

	class remove_refs : public TestWithParam<remove_refs_test> {};

	TEST_P(remove_refs, rm) {
		auto const& [_, ref_info, expected, steps] = GetParam();

		{
			std::error_code ec{};
			path_info::op(steps, ec);
			ASSERT_FALSE(ec) << "   Error: " << ec.message() << " ("
			                 << ec.category().name() << ')';
		}

		auto refs =
		    cov::references::make_refs(setup::test_dir() / steps.front().name);
		ASSERT_TRUE(refs);

		auto const ref = ref_info.create_with(refs);
		ASSERT_TRUE(ref);

		auto const actual = refs->remove_ref(ref);
		ASSERT_EQ(expected, actual) << "  exp. message: " << expected.message()
		                            << "\n  act. message: " << actual.message();
	}

	static remove_refs_test const removes[] = {
	    {
	        "remove HEAD"sv,
	        {"HEAD"sv, ID},
	        git::make_error_code(git::errc::error),
	        make_setup(remove_all("never-HEAD"sv),
	                   create_directories("never-HEAD"sv),
	                   touch("never-HEAD/HEAD"sv,
	                         "112233445566778899aabbccddeeff00gg345678\n"sv)),
	    },
	    {
	        "mismatched types"sv,
	        {"refs/heads/symbolic"sv, "tags/not-an-oid"sv},
	        git::make_error_code(git::errc::modified),
	        make_setup(remove_all("mismatched-types"sv),
	                   create_directories("mismatched-types"sv),
	                   touch("mismatched-types/refs/heads/symbolic"sv, ID_)),
	    },
	    {
	        "mismatched oids"sv,
	        {"refs/heads/main"sv, ID},
	        git::make_error_code(git::errc::modified),
	        make_setup(remove_all("mismatched-oids"sv),
	                   create_directories("mismatched-oids"sv),
	                   touch("mismatched-oids/refs/heads/main"sv,
	                         "112233445566778899aabbccddeeff0087654321\n"sv)),
	    },
	    {
	        "mismatched symlinks"sv,
	        {"refs/tags/tag"sv, "old value"},
	        git::make_error_code(git::errc::modified),
	        make_setup(remove_all("mismatched-symlinks"sv),
	                   create_directories("mismatched-symlinks"sv),
	                   touch("mismatched-symlinks/refs/tags/tag"sv,
	                         "ref: new value\n"sv)),
	    },
	    {
	        "bad ref"sv,
	        {"refs/tags/faulty"sv, "ok-ish"sv},
	        git::make_error_code(git::errc::notfound),
	        make_setup(remove_all("bad-ref"sv),
	                   create_directories("bad-ref"sv),
	                   touch("bad-ref/refs/tags/faulty"sv,
	                         "112233445566778899aabbccddeeff00gg345678\n"sv)),
	    },
	    {
	        "invalid ref"sv,
	        {"refs/tags/not:ok"sv, "ok-ish"sv},
	        git::make_error_code(git::errc::invalidspec),
	        make_setup(remove_all("invalid-ref"sv),
	                   create_directories("invalid-ref"sv),
	                   touch("invalid-ref/refs/tags/tag"sv, "ref: ok-ish\n"sv)),
	    },
	    {
	        "bad ref contents"sv,
	        {"refs/tags/bad"sv, "ok-ish"sv},
	        git::make_error_code(git::errc::notfound),
	        make_setup(remove_all("bad-ref"sv),
	                   create_directories("bad-ref"sv),
	                   touch("bad-ref/refs/tags/bad"sv,
	                         "112233445566778899aabbccddeeff00gg345678\n"sv)),
	    },
	    {
	        "nothing found"sv,
	        {"refs/tags/tag"sv, "ok-ish"sv},
	        git::make_error_code(git::errc::notfound),
	        make_setup(remove_all("nothing found"sv),
	                   create_directories("nothing found"sv),
	                   touch("nothing found/refs/tags/another-tag"sv, ID_)),
	    },
	    {
	        "something found"sv,
	        {"refs/tags/tag"sv, ID},
	        {},
	        make_setup(remove_all("something found"sv),
	                   create_directories("something found"sv),
	                   touch("something found/refs/tags/tag"sv, ID_)),
	    },
	    {
	        "a directory"sv,
	        {"refs/tags/dir"sv, ID},
	        git::make_error_code(git::errc::notfound),
	        make_setup(remove_all("a directory"sv),
	                   create_directories("a directory"sv),
	                   touch("a directory/refs/tags/dir/tag"sv, ID_)),
	    },
	};

	INSTANTIATE_TEST_SUITE_P(named, remove_refs, ::testing::ValuesIn(removes));

	struct copy_refs_test {
		std::string_view name;
		testing::ref_info ref_info;
		struct copy_info {
			std::string_view new_name;
			bool as_branch{true};
			bool force{false};
		} copy_info;
		std::error_code expected;
		std::vector<path_info> steps;

		friend std::ostream& operator<<(std::ostream& out,
		                                copy_refs_test const& param) {
			return out << param.name;
		}
	};

	class copy_refs : public TestWithParam<copy_refs_test> {};

	TEST_P(copy_refs, cp) {
		auto const& [_, ref_info, copy_info, expected, steps] = GetParam();

		{
			std::error_code ec{};
			path_info::op(steps, ec);
			ASSERT_FALSE(ec) << "   Error: " << ec.message() << " ("
			                 << ec.category().name() << ')';
		}

		auto refs =
		    cov::references::make_refs(setup::test_dir() / steps.front().name);
		ASSERT_TRUE(refs);

		auto const ref = ref_info.create_with(refs);
		ASSERT_TRUE(ref);

		std::error_code actual{};
		auto const new_ref =
		    refs->copy_ref(ref, copy_info.new_name, copy_info.as_branch,
		                   copy_info.force, actual);

		ASSERT_EQ(expected, actual) << "  exp. message: " << expected.message()
		                            << "\n  act. message: " << actual.message();
		ASSERT_EQ(!expected, !!new_ref);
	}

	static copy_refs_test const copies[] = {
	    {
	        "HEAD-to-branch"sv,
	        {"HEAD"sv, ID},
	        {"branch"sv},
	        {},
	        make_setup(remove_all("HEAD-to-branch"sv),
	                   create_directories("HEAD-to-branch"sv),
	                   touch("HEAD-to-branch/HEAD"sv, ID_)),
	    },
	    {
	        "duplicate-branch"sv,
	        {"HEAD"sv, ID},
	        {"main"sv},
	        git::make_error_code(git::errc::exists),
	        make_setup(
	            remove_all("duplicate-branch"sv),
	            create_directories("duplicate-branch"sv),
	            touch("duplicate-branch/HEAD"sv, "ref: refs/heads/main\n"),
	            touch("duplicate-branch/refs/heads/main"sv, ID_)),
	    },
	    {
	        "duplicate-branch (force)"sv,
	        {"HEAD"sv, ID},
	        {.new_name = "main"sv, .force = true},
	        {},
	        make_setup(remove_all("duplicate-branch-force"sv),
	                   create_directories("duplicate-branch-force"sv),
	                   touch("duplicate-branch-force/HEAD"sv,
	                         "ref: refs/heads/main\n"),
	                   touch("duplicate-branch-force/refs/heads/main"sv, ID_)),
	    },
	    {
	        "duplicate-tag"sv,
	        {"HEAD"sv, ID},
	        {.new_name = "main"sv, .as_branch = false},
	        {},
	        make_setup(remove_all("duplicate-tag"sv),
	                   create_directories("duplicate-tag"sv),
	                   touch("duplicate-tag/HEAD"sv, "ref: refs/heads/main\n"),
	                   touch("duplicate-tag/refs/heads/main"sv, ID_)),
	    },
	    {
	        "unborn"sv,
	        {"HEAD"sv, "refs/heads/main"},
	        {.new_name = "task/cov-branches"sv, .as_branch = false},
	        git::make_error_code(git::errc::unbornbranch),
	        make_setup(remove_all("unborn"sv),
	                   create_directories("unborn"sv),
	                   touch("unborn/HEAD"sv, "ref: refs/heads/main\n")),
	    },
	    {
	        "invalid-spec"sv,
	        {"HEAD"sv, ID},
	        {.new_name = "task/cov:branches"sv, .as_branch = false},
	        git::make_error_code(git::errc::invalidspec),
	        make_setup(remove_all("invalid-spec"sv),
	                   create_directories("invalid-spec"sv),
	                   touch("invalid-spec/HEAD"sv, ID_),
	                   touch("invalid-spec/refs/heads/main"sv, ID_)),
	    },
	    {
	        "indirect"sv,
	        {"HEAD"sv, "refs/heads/main"},
	        {.new_name = "copy"sv},
	        {},
	        make_setup(remove_all("indirect"sv),
	                   create_directories("indirect"sv),
	                   touch("indirect/HEAD"sv, "ref: refs/heads/main\n"),
	                   touch("indirect/refs/heads/main"sv, ID_)),
	    },
	};

	INSTANTIATE_TEST_SUITE_P(named, copy_refs, ::testing::ValuesIn(copies));
}  // namespace cov::testing

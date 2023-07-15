// Copyright (c) 2022 Marcin Zdun
// This code is licensed under MIT license (see LICENSE for details)

#include "setup.hh"
#include <cov/git2/global.hh>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <tar.hh>

namespace git::testing::setup {
	using namespace ::std::literals;

	namespace {
		class test_globals {
		public:
			static test_globals& get() noexcept {
				static test_globals self{};
				return self;
			}

			void enter() {
				if (!counter++) setup_test_env();
			}

			void leave() {
				if (!--counter) teardown_test_env();
			}

		private:
			static void setup_test_env();
			static void teardown_test_env();

			init thread{};
			int counter{};
		};
		std::filesystem::path get_test_dir() {
			static constexpr auto repos =
			    "repos-8108d2eb7411beefc3f865a5ef918403cf37e70b"sv;
			std::error_code ec{};
			auto const temp = std::filesystem::temp_directory_path(ec);
			if (ec) {
				printf("Running tests from %.*s\n",
				       static_cast<int>(repos.length()), repos.data());
				return repos;
			}
			printf("Running tests from %s\n", get_path(temp / repos).c_str());
			return temp / repos;
		}
	};  // namespace

	test_initializer::test_initializer() { test_globals::get().enter(); }
	test_initializer::~test_initializer() { test_globals::get().leave(); }

	std::filesystem::path test_dir() {
		static std::filesystem::path dirname = get_test_dir();
		return dirname;
	}

	git::repository open_repo(std::error_code& ec) {
		return git::repository::open(test_dir() / "bare.git/"sv, ec);
	}

	USE_TAR

	namespace {
		namespace file {
			static constexpr auto text_1 =
			    "[core]\n"
			    "       repositoryformatversion = 0\n"
			    "       filemode = false\n"
			    "       bare = true\n"
			    "       ignorecase = true\n"sv;
			static constexpr auto text_2 = "ref: refs/heads/main\n"sv;
			static constexpr auto text_3 =
			    "b3f67d63bd1c87427805eba4aa028bfa43587f78\n"sv;
			static constexpr auto text_4 =
			    "[submodule \"bare\"]\n"
			    "	path = bare\n"
			    "	url = ../bare.git\n"sv;
			static constexpr auto text_5 =
			    "[core]\n"
			    "       repositoryformatversion = 0\n"
			    "       filemode = false\n"
			    "       bare = false\n"
			    "       logallrefupdates = true\n"
			    "       ignorecase = true\n"
			    "[submodule \"bare\"]\n"
			    "       url = /tmp/repos/bare.git\n"
			    "       active = true\n"sv;
			static constexpr auto text_6 =
			    "[core]\n"
			    "       repositoryformatversion = 0\n"
			    "       filemode = true\n"
			    "       bare = false\n"
			    "       logallrefupdates = true\n"
			    "       worktree = ../../../bare\n"
			    "[remote \"origin\"]\n"
			    "       url = /tmp/repos/bare.git\n"
			    "       fetch = +refs/heads/*:refs/remotes/origin/*\n"
			    "[branch \"main\"]\n"
			    "       remote = origin\n"
			    "       merge = refs/heads/main\n"sv;
			static constexpr auto text_7 = ""sv;
			static constexpr auto text_8 =
			    "# pack-refs with: peeled fully-peeled sorted \n"
			    "ed631389fc343f7788bf414c2b3e77749a15deb6 refs/remotes/origin/main\n"sv;
			static constexpr auto text_9 = "ref: refs/remotes/origin/main\n"sv;
			static constexpr auto text_10 =
			    "b78559c626ce713277ed5c9bac57e20785091000\n"sv;
			static constexpr auto text_11 = "gitdir: ../.git/modules/bare\n"sv;
			static constexpr auto text_12 =
			    "# Testing repos\n"
			    "\n"
			    "Change from this commit\n"sv;
			static constexpr auto text_13 =
			    "first line\r\n"
			    "second line\r\n"sv;

			static constexpr unsigned char binary_1[] = {
			    0x78, 0x01, 0x4b, 0xca, 0xc9, 0x4f, 0x52, 0x30, 0x31, 0x64,
			    0x50, 0x56, 0x08, 0x49, 0x2d, 0x2e, 0xc9, 0xcc, 0x4b, 0x57,
			    0x28, 0x4a, 0x2d, 0xc8, 0x2f, 0xe6, 0xe2, 0x72, 0xce, 0x48,
			    0xcc, 0x4b, 0x4f, 0x2d, 0x56, 0x48, 0x2b, 0xca, 0xcf, 0x55,
			    0x28, 0xc9, 0xc8, 0x2c, 0x56, 0x48, 0xce, 0xcf, 0xcd, 0xcd,
			    0x2c, 0x01, 0x00, 0x89, 0x0c, 0x10, 0xb1};

			static constexpr unsigned char binary_2[] = {
			    0x78, 0x01, 0x2b, 0x29, 0x4a, 0x4d, 0x55, 0x30, 0x36,
			    0x67, 0x30, 0x34, 0x30, 0x30, 0x33, 0x31, 0x51, 0x08,
			    0x72, 0x75, 0x74, 0xf1, 0x75, 0xd5, 0xcb, 0x4d, 0x61,
			    0x98, 0x18, 0x9c, 0x6a, 0xe0, 0xff, 0xff, 0xd0, 0xd9,
			    0xfe, 0xa7, 0x3b, 0x1e, 0x6d, 0xea, 0xf9, 0xf3, 0xd3,
			    0xff, 0xce, 0xcf, 0x7b, 0x00, 0x4f, 0xf9, 0x13, 0xd0};

			static constexpr unsigned char binary_3[] = {
			    0x78, 0x01, 0x2b, 0x29, 0x4a, 0x4d, 0x55, 0x30, 0x36,
			    0x67, 0x30, 0x34, 0x30, 0x30, 0x33, 0x31, 0x51, 0x08,
			    0x72, 0x75, 0x74, 0xf1, 0x75, 0xd5, 0xcb, 0x4d, 0x61,
			    0xb8, 0x11, 0x93, 0xa5, 0xbc, 0x9c, 0x61, 0x95, 0xc2,
			    0xdf, 0xe5, 0xe7, 0xd7, 0xf5, 0xad, 0x9a, 0xd7, 0xf0,
			    0x4a, 0xe9, 0xee, 0x03, 0x00, 0x3e, 0xdc, 0x11, 0xa9};

			static constexpr unsigned char binary_4[] = {
			    0x78, 0x01, 0x4b, 0xca, 0xc9, 0x4f, 0x52, 0x30, 0x31, 0x64,
			    0x50, 0x56, 0x08, 0x49, 0x2d, 0x2e, 0xc9, 0xcc, 0x4b, 0x57,
			    0x28, 0x4a, 0x2d, 0xc8, 0x2f, 0xe6, 0xe2, 0x72, 0xce, 0x48,
			    0xcc, 0x4b, 0x4f, 0x55, 0x48, 0x2b, 0xca, 0xcf, 0x55, 0x28,
			    0xc9, 0xc8, 0x2c, 0x56, 0x48, 0xce, 0xcf, 0xcd, 0xcd, 0x2c,
			    0xe1, 0x02, 0x00, 0x87, 0x55, 0x10, 0x48};

			static constexpr unsigned char binary_5[] = {
			    0x78, 0x01, 0x2b, 0x29, 0x4a, 0x4d, 0x55, 0x30, 0x36,
			    0x67, 0x30, 0x34, 0x30, 0x30, 0x33, 0x31, 0x51, 0x08,
			    0x72, 0x75, 0x74, 0xf1, 0x75, 0xd5, 0xcb, 0x4d, 0x61,
			    0x10, 0xeb, 0xe2, 0xb9, 0x6e, 0xb7, 0x59, 0x63, 0x15,
			    0xbf, 0xf6, 0x9d, 0x25, 0x4e, 0xcd, 0x5d, 0xa2, 0x6f,
			    0xa3, 0xbd, 0x56, 0x03, 0x00, 0x25, 0x51, 0x0e, 0xd8};

			static constexpr unsigned char binary_6[] = {
			    0x78, 0x01, 0x6d, 0x8e, 0xcb, 0x4a, 0x04, 0x31, 0x10, 0x45,
			    0x5d, 0xe7, 0x2b, 0x6a, 0x2f, 0x48, 0x2a, 0xaf, 0x4a, 0x60,
			    0x10, 0x1f, 0x3b, 0xff, 0x22, 0x8f, 0x8a, 0xdd, 0x83, 0x9d,
			    0x34, 0x31, 0x22, 0xfe, 0xbd, 0x8d, 0x33, 0xba, 0x72, 0x77,
			    0x39, 0x70, 0x0e, 0x37, 0xf7, 0x6d, 0x5b, 0x27, 0x28, 0x27,
			    0x6f, 0xe6, 0x60, 0x06, 0xf4, 0xc1, 0xeb, 0x22, 0x55, 0x30,
			    0xd9, 0x1a, 0xe9, 0x0c, 0x06, 0xac, 0xd9, 0x55, 0x25, 0x13,
			    0x5a, 0x1f, 0x13, 0x22, 0x19, 0xca, 0x8a, 0x8a, 0xd8, 0xe3,
			    0xe0, 0x36, 0x81, 0x8b, 0xd3, 0xa8, 0x7d, 0xa8, 0x59, 0x1b,
			    0x5d, 0x89, 0xbc, 0x4f, 0xd5, 0xa0, 0xc9, 0x2a, 0x69, 0x26,
			    0x22, 0x13, 0x22, 0xda, 0xc2, 0xc9, 0x89, 0xf8, 0x31, 0x97,
			    0x3e, 0xe0, 0xa5, 0x2f, 0xad, 0x7d, 0xc1, 0xe3, 0xbe, 0xbf,
			    0xf1, 0x3b, 0x73, 0x81, 0xd3, 0xf9, 0x87, 0x3c, 0xfc, 0x82,
			    0xbb, 0xdc, 0xb7, 0x7b, 0x40, 0x67, 0x8f, 0x96, 0x91, 0xd6,
			    0xc1, 0xad, 0x54, 0x52, 0x8a, 0x83, 0x1e, 0x3f, 0x27, 0x0f,
			    0x78, 0xfe, 0x5b, 0x4f, 0x7d, 0xc2, 0x29, 0xf5, 0xf9, 0xbf,
			    0x8b, 0x92, 0xae, 0xae, 0xb8, 0x28, 0xf0, 0xb9, 0xce, 0x05,
			    0xae, 0x3f, 0x62, 0x2b, 0x70, 0x69, 0x1e, 0xc9, 0xb2, 0xd6,
			    0xca, 0x63, 0x6d, 0xaf, 0xe2, 0x1b, 0xc4, 0x37, 0x53, 0xb7};

			static constexpr unsigned char binary_7[] = {
			    0x78, 0x01, 0x4b, 0xca, 0xc9, 0x4f, 0x52, 0x30,
			    0x34, 0x63, 0x50, 0x56, 0x08, 0x49, 0x2d, 0x2e,
			    0xc9, 0xcc, 0x4b, 0x57, 0x28, 0x4a, 0x2d, 0xc8,
			    0x2f, 0xe6, 0x02, 0x00, 0x5b, 0x5a, 0x07, 0x9b};

			static constexpr unsigned char binary_8[] = {
			    0x78, 0x01, 0x6d, 0x8e, 0xcb, 0x4a, 0x04, 0x31, 0x10, 0x45,
			    0x5d, 0xe7, 0x2b, 0x6a, 0x2f, 0x48, 0x2a, 0x95, 0x27, 0x0c,
			    0xa2, 0xce, 0xce, 0xbf, 0xc8, 0xa3, 0x32, 0xdd, 0x62, 0x27,
			    0x4d, 0x8c, 0x88, 0x7f, 0x6f, 0x23, 0xa3, 0x6e, 0xdc, 0x5d,
			    0x0e, 0xdc, 0xc3, 0xc9, 0x7d, 0xdb, 0xd6, 0x09, 0xca, 0xe2,
			    0xcd, 0x1c, 0xcc, 0x10, 0x73, 0xd2, 0x4e, 0x2a, 0x4c, 0x05,
			    0x25, 0x4b, 0x32, 0x8c, 0xde, 0x16, 0xb4, 0xe8, 0x31, 0x67,
			    0x4f, 0xd1, 0x25, 0x4f, 0x26, 0x49, 0x74, 0x62, 0x8f, 0x83,
			    0xdb, 0x04, 0x2e, 0x96, 0x90, 0x7c, 0xa8, 0x99, 0x34, 0x55,
			    0xe7, 0xbc, 0x4f, 0x55, 0xa3, 0xce, 0x2a, 0x11, 0x3b, 0xe7,
			    0x74, 0x88, 0x68, 0x0a, 0x27, 0x2b, 0xe2, 0xfb, 0x5c, 0xfa,
			    0x80, 0xe7, 0xbe, 0xb4, 0xf6, 0x09, 0x8f, 0xfb, 0xfe, 0xca,
			    0x6f, 0xcc, 0x05, 0x4e, 0x2f, 0xdf, 0xe4, 0xe1, 0x07, 0xdc,
			    0xe5, 0xbe, 0xdd, 0x03, 0x5a, 0x73, 0xb8, 0x08, 0x43, 0x80,
			    0x5b, 0xa9, 0xa4, 0x14, 0x07, 0x3d, 0x3a, 0x27, 0x0f, 0x38,
			    0xff, 0xae, 0xa7, 0x3e, 0xe1, 0x94, 0xfa, 0xfc, 0xff, 0xab,
			    0x82, 0xb9, 0x7e, 0xc5, 0x79, 0x89, 0xed, 0xc2, 0xf0, 0xb1,
			    0xce, 0x05, 0xae, 0x1d, 0xb1, 0x15, 0xf8, 0x73, 0x96, 0xb5,
			    0x56, 0x1e, 0x6b, 0xbb, 0x88, 0x2f, 0x94, 0x31, 0x54, 0x8e};

			static constexpr unsigned char binary_9[] = {
			    0x78, 0x01, 0x9d, 0x8d, 0x41, 0x0a, 0xc2, 0x30, 0x10, 0x00,
			    0x3d, 0xe7, 0x15, 0x7b, 0x17, 0x64, 0x93, 0x36, 0xcd, 0x06,
			    0x8a, 0x78, 0xf5, 0xe0, 0x07, 0xbc, 0x65, 0x9b, 0xd4, 0x2e,
			    0x98, 0x14, 0xea, 0xf6, 0xe2, 0xeb, 0xed, 0x1b, 0x3c, 0x0d,
			    0x0c, 0x0c, 0x33, 0xad, 0xb5, 0x8a, 0x82, 0x25, 0x3c, 0xe9,
			    0x56, 0x0a, 0x04, 0xcb, 0x1c, 0x22, 0xa6, 0xbe, 0x50, 0x61,
			    0x74, 0x4c, 0x6e, 0x8e, 0x3d, 0xe5, 0x99, 0x6d, 0x97, 0x63,
			    0x37, 0x44, 0x0e, 0x03, 0x65, 0xec, 0x83, 0x49, 0xbb, 0x2e,
			    0xeb, 0x06, 0x8f, 0xb4, 0x4d, 0xd2, 0xe0, 0x99, 0xf7, 0x06,
			    0x63, 0xfd, 0x1e, 0xb8, 0x55, 0xc9, 0x4d, 0x5e, 0x8b, 0xb2,
			    0xe8, 0xe7, 0x32, 0xad, 0xf5, 0x0a, 0x76, 0xf0, 0xde, 0x07,
			    0x6f, 0x89, 0xe0, 0x8c, 0x0e, 0xd1, 0x1c, 0xf6, 0xb8, 0x6a,
			    0xf9, 0xb7, 0x37, 0xf7, 0x26, 0x2a, 0xe9, 0x6d, 0x7e, 0x47,
			    0xfc, 0x3b, 0x35};

			static constexpr unsigned char binary_10[] = {
			    0x44, 0x49, 0x52, 0x43, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00,
			    0x00, 0x03, 0x62, 0xcf, 0xc2, 0xc9, 0x0b, 0xfc, 0xb7, 0xcc,
			    0x62, 0xcf, 0xc6, 0xb5, 0x22, 0x9b, 0xea, 0x48, 0x00, 0x00,
			    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x81, 0xa4,
			    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			    0x00, 0x33, 0xbc, 0xa9, 0xd5, 0xf2, 0xa5, 0x00, 0x92, 0xf1,
			    0xad, 0x2f, 0x52, 0xd0, 0x3b, 0xde, 0xd3, 0xbd, 0x9d, 0x2b,
			    0x5e, 0xdd, 0x00, 0x0b, 0x2e, 0x67, 0x69, 0x74, 0x6d, 0x6f,
			    0x64, 0x75, 0x6c, 0x65, 0x73, 0x00, 0x00, 0x00, 0x00, 0x00,
			    0x00, 0x00, 0x62, 0xcf, 0xc2, 0xc9, 0x0b, 0xfc, 0xb7, 0xcc,
			    0x62, 0xcf, 0xc2, 0xc9, 0x0c, 0x0b, 0xf6, 0x88, 0x00, 0x00,
			    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x00,
			    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			    0x00, 0x00, 0xb3, 0xf6, 0x7d, 0x63, 0xbd, 0x1c, 0x87, 0x42,
			    0x78, 0x05, 0xeb, 0xa4, 0xaa, 0x02, 0x8b, 0xfa, 0x43, 0x58,
			    0x7f, 0x78, 0x00, 0x04, 0x62, 0x61, 0x72, 0x65, 0x00, 0x00,
			    0x00, 0x00, 0x00, 0x00, 0x62, 0xcf, 0xc2, 0xc9, 0x0c, 0x0b,
			    0xf6, 0x88, 0x62, 0xcf, 0xc6, 0x9d, 0x02, 0xf9, 0x84, 0x70,
			    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			    0x81, 0xa4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			    0x00, 0x00, 0x00, 0x19, 0x42, 0xd6, 0xab, 0x78, 0x98, 0x30,
			    0x1b, 0x32, 0xaa, 0xd7, 0x01, 0x91, 0xb3, 0x0e, 0xff, 0x94,
			    0xe7, 0x3a, 0x29, 0x34, 0x00, 0x0d, 0x73, 0x75, 0x62, 0x64,
			    0x69, 0x72, 0x2f, 0x61, 0x2d, 0x66, 0x69, 0x6c, 0x65, 0x00,
			    0x00, 0x00, 0x00, 0x00, 0x54, 0x52, 0x45, 0x45, 0x00, 0x00,
			    0x00, 0x38, 0x00, 0x33, 0x20, 0x31, 0x0a, 0x0c, 0x86, 0x8b,
			    0x61, 0x27, 0x95, 0x57, 0x57, 0x36, 0xa2, 0xda, 0x37, 0xc1,
			    0xcf, 0x9d, 0x87, 0x7a, 0x59, 0x47, 0x7e, 0x73, 0x75, 0x62,
			    0x64, 0x69, 0x72, 0x00, 0x31, 0x20, 0x30, 0x0a, 0x56, 0x18,
			    0xe1, 0x71, 0x82, 0xd4, 0x90, 0x46, 0x37, 0x31, 0xfa, 0x55,
			    0xd3, 0xc9, 0x35, 0x18, 0xae, 0x7b, 0xc2, 0x27, 0x46, 0x53,
			    0x4d, 0x4e, 0x00, 0x00, 0x00, 0x4e, 0x00, 0x00, 0x00, 0x02,
			    0x62, 0x75, 0x69, 0x6c, 0x74, 0x69, 0x6e, 0x3a, 0x30, 0x2e,
			    0x36, 0x30, 0x36, 0x36, 0x38, 0x2e, 0x32, 0x30, 0x32, 0x32,
			    0x30, 0x37, 0x31, 0x34, 0x54, 0x30, 0x37, 0x31, 0x37, 0x30,
			    0x34, 0x2e, 0x39, 0x34, 0x35, 0x33, 0x31, 0x30, 0x5a, 0x3a,
			    0x34, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x02,
			    0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00,
			    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
			    0x00, 0x00, 0x00, 0x00, 0xe9, 0xb5, 0x51, 0xa9, 0xd3, 0x8d,
			    0x34, 0xbb, 0x8d, 0x27, 0x3c, 0x18, 0x8e, 0x38, 0xfe, 0x73,
			    0x07, 0xec, 0x90, 0x0d};

			static constexpr unsigned char binary_11[] = {
			    0x44, 0x49, 0x52, 0x43, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00,
			    0x00, 0x01, 0x62, 0xcf, 0xc2, 0xc9, 0x0c, 0x0b, 0xf6, 0x88,
			    0x62, 0xcf, 0xc6, 0xca, 0x21, 0x1e, 0x6d, 0xbc, 0x00, 0x00,
			    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x81, 0xa4,
			    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			    0x00, 0x29, 0x91, 0x53, 0x65, 0x30, 0x4f, 0xff, 0xc2, 0xcd,
			    0x8f, 0xe5, 0xb8, 0xe2, 0xb2, 0x8c, 0xfc, 0xf9, 0x4f, 0xdc,
			    0xf9, 0xde, 0x00, 0x09, 0x52, 0x45, 0x41, 0x44, 0x4d, 0x45,
			    0x2e, 0x6d, 0x64, 0x00, 0x54, 0x52, 0x45, 0x45, 0x00, 0x00,
			    0x00, 0x19, 0x00, 0x31, 0x20, 0x30, 0x0a, 0x18, 0x98, 0x3d,
			    0x02, 0x94, 0xc5, 0x40, 0x64, 0x19, 0x1f, 0xc6, 0xf2, 0x0b,
			    0x15, 0x8a, 0xb1, 0x17, 0x47, 0xc2, 0x7d, 0x46, 0x53, 0x4d,
			    0x4e, 0x00, 0x00, 0x00, 0x46, 0x00, 0x00, 0x00, 0x02, 0x62,
			    0x75, 0x69, 0x6c, 0x74, 0x69, 0x6e, 0x3a, 0x30, 0x2e, 0x36,
			    0x36, 0x39, 0x38, 0x38, 0x2e, 0x32, 0x30, 0x32, 0x32, 0x30,
			    0x37, 0x31, 0x34, 0x54, 0x30, 0x37, 0x31, 0x37, 0x30, 0x34,
			    0x2e, 0x39, 0x38, 0x36, 0x33, 0x31, 0x30, 0x5a, 0x3a, 0x33,
			    0x00, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00,
			    0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			    0x00, 0x00, 0x00, 0x00, 0x00, 0xdd, 0x66, 0x75, 0x88, 0x2c,
			    0x20, 0xc4, 0x1d, 0xa2, 0xc9, 0x9f, 0x8e, 0x88, 0x9b, 0xc1,
			    0x39, 0x87, 0x96, 0xa3, 0x21};

			static constexpr unsigned char binary_12[] = {
			    0x78, 0x01, 0x9d, 0xcf, 0x49, 0x6a, 0x03, 0x31, 0x10, 0x85,
			    0xe1, 0xac, 0x75, 0x8a, 0xda, 0x1b, 0x8c, 0x4a, 0x2a, 0x4d,
			    0x60, 0x4c, 0x8c, 0x77, 0xbe, 0x85, 0x86, 0x52, 0xba, 0x83,
			    0x5b, 0xdd, 0x74, 0x14, 0x82, 0x6f, 0xef, 0xc6, 0xc3, 0x05,
			    0xb2, 0xfd, 0x17, 0x1f, 0xef, 0xe5, 0x79, 0x9a, 0xc6, 0x0e,
			    0xca, 0xda, 0x8f, 0xbe, 0x32, 0x03, 0xfa, 0xe0, 0x75, 0x91,
			    0x2a, 0x50, 0x36, 0x24, 0x2d, 0x61, 0xc0, 0x9a, 0x6d, 0x55,
			    0x32, 0xa1, 0xf1, 0x31, 0x21, 0x3a, 0x72, 0x59, 0xb9, 0x22,
			    0x96, 0xb8, 0x72, 0xeb, 0xc0, 0xc5, 0x6a, 0xd4, 0x3e, 0xd4,
			    0xac, 0x49, 0x57, 0xe7, 0xbc, 0x4f, 0x95, 0x90, 0xb2, 0x4a,
			    0x9a, 0x9d, 0x73, 0x14, 0x22, 0x9a, 0xc2, 0xc9, 0x8a, 0xf8,
			    0xdb, 0x87, 0x79, 0x85, 0xcb, 0x3c, 0xb4, 0x76, 0x83, 0xd3,
			    0xb2, 0x5c, 0xf9, 0x87, 0xb9, 0xc0, 0xe1, 0xfb, 0x51, 0x3e,
			    0xdf, 0x61, 0x9f, 0xe7, 0xe9, 0x08, 0x68, 0xcd, 0x66, 0x91,
			    0x34, 0x16, 0x76, 0x52, 0x49, 0x29, 0xb6, 0xba, 0xed, 0xec,
			    0xfc, 0x7f, 0x41, 0x9c, 0x9f, 0x57, 0xff, 0xc6, 0x3e, 0xc0,
			    0x6b, 0x4d, 0x6c, 0x05, 0x9e, 0xf2, 0x06, 0x97, 0xb1, 0x56,
			    0x5e, 0xc7, 0xf6, 0x25, 0xee, 0xf4, 0xd1, 0x56, 0x41};

			static constexpr unsigned char binary_13[] = {
			    0x78, 0x01, 0x9d, 0xce, 0x4d, 0x6a, 0xc3, 0x30, 0x10, 0x40,
			    0xe1, 0xac, 0x75, 0x8a, 0xd9, 0x17, 0x82, 0xc6, 0x23, 0x6b,
			    0x64, 0x08, 0xa1, 0x25, 0xbb, 0xdc, 0x42, 0x3f, 0xa3, 0xd8,
			    0x21, 0x96, 0x8c, 0xab, 0x52, 0x72, 0xfb, 0x98, 0xd2, 0x90,
			    0x7d, 0xb6, 0x6f, 0xf1, 0xf1, 0x62, 0x9d, 0xe7, 0xa9, 0x41,
			    0x67, 0x79, 0xd7, 0x56, 0x11, 0xf0, 0x31, 0x18, 0xd6, 0x1d,
			    0x86, 0x84, 0x5a, 0x34, 0xf5, 0x82, 0xce, 0x26, 0xb4, 0xe8,
			    0x30, 0x46, 0x47, 0x9e, 0x83, 0xa3, 0x3e, 0x68, 0x64, 0xb5,
			    0xf8, 0x55, 0x4a, 0x03, 0x49, 0x96, 0x90, 0xdc, 0x90, 0x23,
			    0x19, 0xca, 0xcc, 0xce, 0x85, 0x6c, 0xd0, 0xc4, 0x2e, 0x90,
			    0x30, 0xb3, 0x19, 0x3c, 0xf6, 0x49, 0x82, 0x55, 0xfe, 0xa7,
			    0x8d, 0x75, 0x85, 0x73, 0x1d, 0x4b, 0xb9, 0xc3, 0xd7, 0xb2,
			    0xdc, 0xe4, 0x5b, 0x24, 0xc1, 0xe1, 0xfa, 0x57, 0x3e, 0x9f,
			    0x61, 0x1f, 0xeb, 0x7c, 0x04, 0xb4, 0xfd, 0x66, 0x11, 0x0e,
			    0x03, 0x7c, 0xe8, 0x4e, 0x6b, 0xb5, 0xd5, 0xed, 0xb3, 0xc9,
			    0xfb, 0x82, 0x3a, 0x8d, 0xbe, 0x5c, 0x04, 0x7e, 0xa7, 0x36,
			    0xc2, 0xff, 0x8d, 0x2f, 0x09, 0x5e, 0x72, 0x9a, 0x72, 0x96,
			    0x75, 0x2a, 0x17, 0xf5, 0x00, 0xca, 0x4a, 0x57, 0x18};

			static constexpr unsigned char binary_14[] = {
			    0x78, 0x01, 0x2b, 0x29, 0x4a, 0x4d, 0x55, 0x30, 0x34, 0x30,
			    0x61, 0x30, 0x34, 0x30, 0x30, 0x33, 0x31, 0x51, 0xd0, 0x4b,
			    0xcf, 0x2c, 0xc9, 0xcd, 0x4f, 0x29, 0xcd, 0x49, 0x2d, 0x66,
			    0xd8, 0xb3, 0xf2, 0xea, 0xa7, 0xa5, 0x0c, 0x93, 0x3e, 0xae,
			    0xd5, 0x0f, 0xba, 0x60, 0x7d, 0xef, 0xf2, 0xde, 0xb9, 0xda,
			    0x71, 0x77, 0x0d, 0xcd, 0x0c, 0x80, 0x40, 0x21, 0x29, 0xb1,
			    0x28, 0x95, 0x61, 0xf3, 0xb7, 0xda, 0xe4, 0xbd, 0x32, 0xed,
			    0x4e, 0x15, 0xac, 0xaf, 0x97, 0xac, 0x62, 0xea, 0xfe, 0xe5,
			    0x1c, 0x51, 0x5f, 0x61, 0x02, 0x96, 0x2e, 0x2e, 0x4d, 0x4a,
			    0xc9, 0x2c, 0x62, 0x08, 0x93, 0x78, 0x58, 0xd8, 0x74, 0x65,
			    0x82, 0x9b, 0xb9, 0xe1, 0xaf, 0xd0, 0xcb, 0x27, 0x4d, 0x25,
			    0xd6, 0x55, 0x1f, 0x52, 0x07, 0x00, 0xd5, 0x7f, 0x2e, 0x34};

			static constexpr unsigned char binary_15[] = {
			    0x78, 0x01, 0x4b, 0xca, 0xc9, 0x4f, 0x52, 0x30, 0x32,
			    0x65, 0x48, 0xcb, 0x2c, 0x2a, 0x2e, 0x51, 0xc8, 0xc9,
			    0xcc, 0x4b, 0xe5, 0xe5, 0x2a, 0x4e, 0x4d, 0xce, 0xcf,
			    0x4b, 0x81, 0x72, 0x00, 0xb6, 0xcc, 0x0a, 0x89};

			static constexpr unsigned char binary_16[] = {
			    0x78, 0x01, 0x2b, 0x29, 0x4a, 0x4d, 0x55, 0x30, 0x36,
			    0x61, 0x30, 0x34, 0x30, 0x30, 0x33, 0x31, 0x51, 0x48,
			    0xd4, 0x4d, 0xcb, 0xcc, 0x49, 0x65, 0x70, 0xba, 0xb6,
			    0xba, 0x62, 0x86, 0x81, 0xb4, 0xd1, 0xaa, 0xeb, 0x8c,
			    0x13, 0x37, 0xf3, 0xfd, 0x9f, 0xf2, 0xdc, 0x4a, 0xd3,
			    0x04, 0x00, 0x19, 0x2e, 0x0e, 0xea};

			static constexpr unsigned char binary_17[] = {
			    0x78, 0x01, 0x2b, 0x29, 0x4a, 0x4d, 0x55, 0x30, 0x34, 0x30,
			    0x61, 0x30, 0x34, 0x30, 0x30, 0x33, 0x31, 0x51, 0xd0, 0x4b,
			    0xcf, 0x2c, 0xc9, 0xcd, 0x4f, 0x29, 0xcd, 0x49, 0x2d, 0x66,
			    0xd8, 0xb3, 0xf2, 0xea, 0xa7, 0xa5, 0x0c, 0x93, 0x3e, 0xae,
			    0xd5, 0x0f, 0xba, 0x60, 0x7d, 0xef, 0xf2, 0xde, 0xb9, 0xda,
			    0x71, 0x77, 0x0d, 0xcd, 0x0c, 0x80, 0x40, 0x21, 0x29, 0xb1,
			    0x28, 0x95, 0xe1, 0x6d, 0xb2, 0x70, 0xe7, 0x1f, 0x13, 0xfb,
			    0xf2, 0x8e, 0xfd, 0x8e, 0x3e, 0xda, 0x76, 0xe5, 0x25, 0xb3,
			    0x44, 0xef, 0x6d, 0x33, 0x01, 0x4b, 0x17, 0x97, 0x26, 0xa5,
			    0x64, 0x16, 0x31, 0x84, 0x49, 0x3c, 0x2c, 0x6c, 0xba, 0x32,
			    0xc1, 0xcd, 0xdc, 0xf0, 0x57, 0xe8, 0xe5, 0x93, 0xa6, 0x12,
			    0xeb, 0xaa, 0x0f, 0xa9, 0x03, 0x00, 0xb1, 0x13, 0x2d, 0x77};

			static constexpr unsigned char binary_18[] = {
			    0x78, 0x01, 0x9d, 0x8d, 0xb1, 0x0a, 0xc2, 0x30, 0x14, 0x45,
			    0x9d, 0xf3, 0x15, 0x6f, 0x17, 0x4a, 0x9a, 0x36, 0x31, 0x05,
			    0x11, 0xdd, 0xed, 0x0f, 0xb8, 0xbd, 0x26, 0xaf, 0x6d, 0x84,
			    0xb4, 0x25, 0x79, 0x5d, 0xfc, 0x7a, 0x03, 0x2e, 0x0e, 0x4e,
			    0x4e, 0x17, 0x0e, 0xf7, 0xdc, 0xeb, 0xd6, 0x18, 0x03, 0x83,
			    0xd2, 0xf6, 0xc0, 0x89, 0x08, 0xb4, 0xa1, 0x96, 0x3a, 0xab,
			    0xac, 0x1d, 0xd1, 0xb5, 0x0e, 0x6d, 0x67, 0x5b, 0xa3, 0xa5,
			    0x1a, 0x25, 0xf9, 0xda, 0x37, 0x76, 0x68, 0x6a, 0xab, 0xa8,
			    0x95, 0x02, 0x77, 0x9e, 0xd7, 0x04, 0x3d, 0x26, 0x17, 0x16,
			    0x78, 0xf8, 0x7d, 0x81, 0x73, 0x7c, 0x95, 0xb8, 0xc6, 0xe0,
			    0x97, 0x30, 0xcd, 0x3c, 0x04, 0xce, 0x95, 0x5b, 0xe3, 0x05,
			    0x6a, 0xa3, 0xb5, 0x51, 0xea, 0x64, 0x34, 0x1c, 0xa5, 0x92,
			    0x52, 0x14, 0x5a, 0x5e, 0x99, 0xfe, 0xf5, 0xc5, 0x0d, 0xf2,
			    0x3e, 0x3c, 0xc9, 0xb1, 0x10, 0x3d, 0xe5, 0x8c, 0x13, 0xc1,
			    0x86, 0x09, 0xa7, 0x84, 0xdb, 0x0c, 0x75, 0xf5, 0x8b, 0xaa,
			    0x42, 0xef, 0x98, 0xf9, 0xab, 0xb8, 0x8e, 0xc0, 0x33, 0x41,
			    0xfc, 0x2c, 0x54, 0xe2, 0x0d, 0x1a, 0x06, 0x54, 0xe7};

			static constexpr unsigned char binary_19[] = {
			    0x78, 0x01, 0x95, 0x8e, 0x4d, 0x0a, 0xc2, 0x30, 0x10, 0x46,
			    0x5d, 0xe7, 0x14, 0xb3, 0x17, 0x24, 0x93, 0xbf, 0x49, 0x40,
			    0x44, 0x74, 0xed, 0x21, 0xd2, 0x64, 0x8a, 0x42, 0x63, 0x4a,
			    0x9a, 0x7a, 0x7e, 0x5b, 0x6f, 0xe0, 0xf6, 0x7d, 0x3c, 0xbe,
			    0x97, 0x6a, 0x29, 0xaf, 0x0e, 0x4a, 0xd1, 0xa1, 0x37, 0x66,
			    0x90, 0xc9, 0x3b, 0x3f, 0x38, 0x54, 0x14, 0xac, 0x25, 0x4b,
			    0xda, 0x45, 0x95, 0xa3, 0xa6, 0x84, 0x69, 0x0c, 0xd9, 0x13,
			    0x45, 0x1b, 0x0c, 0x11, 0x8b, 0x39, 0x36, 0x7e, 0x77, 0x20,
			    0x64, 0xc7, 0x8c, 0xfb, 0x9a, 0x28, 0x67, 0x3f, 0xaa, 0x91,
			    0xd0, 0xa0, 0x4d, 0x9a, 0xd9, 0x44, 0x1d, 0x0d, 0x62, 0x0e,
			    0xc9, 0x5b, 0x11, 0xd7, 0xfe, 0xac, 0x0d, 0xee, 0xbf, 0xbf,
			    0xce, 0x0d, 0x6e, 0xb5, 0xc3, 0x79, 0xa8, 0xfd, 0x3a, 0xcf,
			    0x13, 0x2f, 0xcc, 0xf9, 0x94, 0x6a, 0xb9, 0x00, 0x3a, 0x4b,
			    0xe4, 0x0d, 0x12, 0xc1, 0x51, 0x2a, 0x29, 0xc5, 0x46, 0xb7,
			    0xc2, 0xdd, 0xf8, 0xdf, 0x15, 0x8f, 0xfa, 0x61, 0x58, 0xd6,
			    0xa1, 0xd4, 0xbc, 0x4e, 0x2c, 0xbe, 0x10, 0x02, 0x48, 0x04};

			static constexpr unsigned char binary_20[] = {
			    0x78, 0x01, 0x4b, 0xca, 0xc9, 0x4f, 0x52, 0x30, 0x35,
			    0x64, 0x88, 0x2e, 0x2e, 0x4d, 0xca, 0xcd, 0x4f, 0x29,
			    0xcd, 0x49, 0x55, 0x50, 0x4a, 0x4a, 0x2c, 0x4a, 0x55,
			    0x8a, 0xe5, 0xe2, 0x2c, 0x48, 0x2c, 0xc9, 0x50, 0xb0,
			    0x55, 0x00, 0x71, 0xb9, 0x38, 0x4b, 0x8b, 0x72, 0x80,
			    0x6c, 0x3d, 0x3d, 0x7d, 0x10, 0x57, 0x2f, 0x3d, 0xb3,
			    0x84, 0x0b, 0x00, 0x2c, 0xe6, 0x12, 0x07};
		}  // namespace file

		constexpr std::string_view subdirs[] = {
		    "bare.git/objects/info"sv,
		    "bare.git/objects/pack"sv,
		    "bare.git/refs/tags"sv,
		    "gitdir/.git/fsmonitor--daemon/cookies"sv,
		    "gitdir/.git/modules/bare/branches"sv,
		    "gitdir/.git/modules/bare/fsmonitor--daemon/cookies"sv,
		    "gitdir/.git/modules/bare/objects/info"sv,
		    "gitdir/.git/modules/bare/objects/pack"sv,
		    "gitdir/.git/modules/bare/refs/tags"sv,
		    "gitdir/.git/objects/info"sv,
		    "gitdir/.git/objects/pack"sv,
		    "gitdir/.git/refs/tags"sv,
		};

		constexpr file::text text[] = {
		    {"bare.git/HEAD"sv, file::text_2},
		    {"bare.git/config"sv, file::text_1},
		    {"bare.git/refs/heads/main"sv, file::text_3},
		    {"gitdir/.git/HEAD"sv, file::text_2},
		    {"gitdir/.git/config"sv, file::text_5},
		    {"gitdir/.git/modules/bare/FETCH_HEAD"sv, file::text_7},
		    {"gitdir/.git/modules/bare/HEAD"sv, file::text_2},
		    {"gitdir/.git/modules/bare/config"sv, file::text_6},
		    {"gitdir/.git/modules/bare/packed-refs"sv, file::text_8},
		    {"gitdir/.git/modules/bare/refs/heads/main"sv, file::text_3},
		    {"gitdir/.git/modules/bare/refs/remotes/origin/HEAD"sv,
		     file::text_9},
		    {"gitdir/.git/modules/bare/refs/remotes/origin/main"sv,
		     file::text_3},
		    {"gitdir/.git/refs/heads/main"sv, file::text_10},
		    {"gitdir/.gitmodules"sv, file::text_4},
		    {"gitdir/bare/.git"sv, file::text_11},
		    {"gitdir/bare/README.md"sv, file::text_12},
		    {"gitdir/subdir/a-file"sv, file::text_13},
		};

		constexpr file::binary binary[] = {
		    {"bare.git/objects/16/8a0cd73eb328aa0f2bdca442838a15ed5b4aab"sv,
		     span(file::binary_1)},
		    {"bare.git/objects/18/983d0294c54064191fc6f20b158ab11747c27d"sv,
		     span(file::binary_2)},
		    {"bare.git/objects/71/bb790a4e8eb02b82f948dfb13d9369b768d047"sv,
		     span(file::binary_3)},
		    {"bare.git/objects/91/5365304fffc2cd8fe5b8e2b28cfcf94fdcf9de"sv,
		     span(file::binary_4)},
		    {"bare.git/objects/ac/b47021bd10e035e186d16181cc83a7b835b017"sv,
		     span(file::binary_5)},
		    {"bare.git/objects/b3/f67d63bd1c87427805eba4aa028bfa43587f78"sv,
		     span(file::binary_6)},
		    {"bare.git/objects/d8/5c6a23a700aa20fda7cfae8eaa9e80ea22dde0"sv,
		     span(file::binary_7)},
		    {"bare.git/objects/db/5edebad084cf005222596533ec0827c1c04337"sv,
		     span(file::binary_8)},
		    {"bare.git/objects/ed/631389fc343f7788bf414c2b3e77749a15deb6"sv,
		     span(file::binary_9)},
		    {"gitdir/.git/index"sv, span(file::binary_10)},
		    {"gitdir/.git/modules/bare/index"sv, span(file::binary_11)},
		    {"gitdir/.git/modules/bare/objects/16/8a0cd73eb328aa0f2bdca442838a15ed5b4aab"sv,
		     span(file::binary_1)},
		    {"gitdir/.git/modules/bare/objects/18/983d0294c54064191fc6f20b158ab11747c27d"sv,
		     span(file::binary_2)},
		    {"gitdir/.git/modules/bare/objects/71/bb790a4e8eb02b82f948dfb13d9369b768d047"sv,
		     span(file::binary_3)},
		    {"gitdir/.git/modules/bare/objects/91/5365304fffc2cd8fe5b8e2b28cfcf94fdcf9de"sv,
		     span(file::binary_4)},
		    {"gitdir/.git/modules/bare/objects/ac/b47021bd10e035e186d16181cc83a7b835b017"sv,
		     span(file::binary_5)},
		    {"gitdir/.git/modules/bare/objects/b3/f67d63bd1c87427805eba4aa028bfa43587f78"sv,
		     span(file::binary_6)},
		    {"gitdir/.git/modules/bare/objects/d8/5c6a23a700aa20fda7cfae8eaa9e80ea22dde0"sv,
		     span(file::binary_7)},
		    {"gitdir/.git/modules/bare/objects/db/5edebad084cf005222596533ec0827c1c04337"sv,
		     span(file::binary_8)},
		    {"gitdir/.git/modules/bare/objects/df/6c2ce69c5e2eb0462d38053de0b1d1822c66ca"sv,
		     span(file::binary_12)},
		    {"gitdir/.git/modules/bare/objects/ed/631389fc343f7788bf414c2b3e77749a15deb6"sv,
		     span(file::binary_9)},
		    {"gitdir/.git/modules/bare/objects/fa/e7e91f589e510edc5f9b541c97188d388588c9"sv,
		     span(file::binary_13)},
		    {"gitdir/.git/objects/0c/868b612795575736a2da37c1cf9d877a59477e"sv,
		     span(file::binary_14)},
		    {"gitdir/.git/objects/42/d6ab7898301b32aad70191b30eff94e73a2934"sv,
		     span(file::binary_15)},
		    {"gitdir/.git/objects/56/18e17182d490463731fa55d3c93518ae7bc227"sv,
		     span(file::binary_16)},
		    {"gitdir/.git/objects/56/e4e98288fac4ca89846502f0ed1d38b3182e40"sv,
		     span(file::binary_17)},
		    {"gitdir/.git/objects/71/e6ee11cf9c7dd8f2f71415c3ee4a3a411d9c85"sv,
		     span(file::binary_18)},
		    {"gitdir/.git/objects/b7/8559c626ce713277ed5c9bac57e20785091000"sv,
		     span(file::binary_19)},
		    {"gitdir/.git/objects/bc/a9d5f2a50092f1ad2f52d03bded3bd9d2b5edd"sv,
		     span(file::binary_20)},
		};
	}  // namespace

	void test_globals::setup_test_env() {
		printf("Setting up test environment\n");

		std::error_code ignore{};
		remove_all(test_dir(), ignore);
		unpack_files(test_dir(), setup::subdirs, setup::text, setup::binary);
	}

	void test_globals::teardown_test_env() {
		printf("Tearing down test environment\n");
		using namespace std::filesystem;
		std::error_code ignore{};
		remove_all(test_dir(), ignore);
	}
}  // namespace git::testing::setup
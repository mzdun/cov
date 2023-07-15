// Copyright (c) 2022 Marcin Zdun
// This code is licensed under MIT license (see LICENSE for details)

#include <cov/git2/blob.hh>
#include <cov/io/report.hh>
#include <cov/io/strings.hh>
#include <cov/io/types.hh>
#include <cov/repository.hh>

namespace cov::io::handlers {
	namespace {
		std::string S(std::string_view value) {
			return {value.data(), value.size()};
		}

		struct signature {
			std::string name;
			std::string email;
		};

		signature S(cov::report::signature_view value) {
			return {.name = S(value.name), .email = S(value.email)};
		}

		struct build_impl : cov::report::build {
			build_impl(git::oid_view build_id,
			           std::string_view props,
			           io::v1::coverage_stats const& stats)
			    : build_id_{build_id.oid()}
			    , props_{props.data(), props.size()}
			    , stats_{stats} {}

			~build_impl() = default;

			git::oid const& build_id() const noexcept { return build_id_; }
			std::string_view props_json() const noexcept { return props_; }
			io::v1::coverage_stats const& stats() const noexcept {
				return stats_;
			}

		private:
			git::oid build_id_{};
			std::string props_{};
			io::v1::coverage_stats stats_{};
		};

		struct impl : counted_impl<cov::report> {
			explicit impl(git::oid_view oid,
			              git::oid_view parent,
			              git::oid_view file_list,
			              git::oid_view commit,
			              std::string_view branch,
			              signature_view author,
			              signature_view committer,
			              std::string_view message,
			              sys_seconds commit_time_utc,
			              sys_seconds add_time_utc,
			              io::v1::coverage_stats const& stats,
			              std::vector<std::unique_ptr<build>>&& builds)
			    : oid_{oid.oid()}
			    , parent_{parent.oid()}
			    , file_list_{file_list.oid()}
			    , commit_{commit.oid()}
			    , branch_{S(branch)}
			    , author_{S(author)}
			    , committer_{S(committer)}
			    , message_{S(message)}
			    , commit_time_utc_{commit_time_utc}
			    , add_time_utc_{add_time_utc}
			    , stats_{stats}
			    , builds_{std::move(builds)} {}

			git::oid const& oid() const noexcept override { return oid_; }
			git::oid const& parent_id() const noexcept override {
				return parent_;
			}
			git::oid const& file_list_id() const noexcept override {
				return file_list_;
			}
			git::oid const& commit_id() const noexcept override {
				return commit_;
			}
			std::string_view branch() const noexcept override {
				return branch_;
			}
			std::string_view author_name() const noexcept override {
				return author_.name;
			}
			std::string_view author_email() const noexcept override {
				return author_.email;
			}
			std::string_view committer_name() const noexcept override {
				return committer_.name;
			}
			std::string_view committer_email() const noexcept override {
				return committer_.email;
			}
			std::string_view message() const noexcept override {
				return message_;
			}
			sys_seconds commit_time_utc() const noexcept override {
				return commit_time_utc_;
			}
			sys_seconds add_time_utc() const noexcept override {
				return add_time_utc_;
			}
			io::v1::coverage_stats const& stats() const noexcept override {
				return stats_;
			}
			std::span<std::unique_ptr<build> const> entries()
			    const noexcept override {
				return builds_;
			}

		private:
			git::oid oid_{};
			git::oid parent_{};
			git::oid file_list_{};
			git::oid commit_{};
			std::string branch_{};
			signature author_{};
			signature committer_{};
			std::string message_{};
			sys_seconds commit_time_utc_{};
			sys_seconds add_time_utc_{};
			io::v1::coverage_stats stats_{};
			std::vector<std::unique_ptr<build>> builds_{};
		};

		constexpr uint32_t uint_32(size_t value) {
			return static_cast<uint32_t>(value &
			                             std::numeric_limits<uint32_t>::max());
		}
		static_assert(sizeof(1ull) > 4);
		static_assert(uint_32(std::numeric_limits<size_t>::max()) ==
		              0xFFFF'FFFF);

		bool load_resources(
		    std::vector<std::unique_ptr<cov::report::build>>& builds,
		    read_stream& in,
		    strings_view_base const& strings,
		    uint32_t entry_size,
		    uint32_t entry_count) {
			std::vector<std::byte> buffer{};
			builds.reserve(entry_count);

			for (uint32_t index = 0; index < entry_count; ++index) {
				if (!in.load(buffer, entry_size)) return {};
				auto const& entry =
				    *reinterpret_cast<v1::report::entry const*>(buffer.data());

				if (!strings.is_valid(entry.propset)) return false;

				builds.push_back(std::make_unique<build_impl>(
				    entry.build, strings.at(entry.propset), entry.stats));
			}

			return true;
		}
	}  // namespace

	ref_ptr<counted> report::load(uint32_t,
	                              uint32_t,
	                              git::oid_view id,
	                              read_stream& in,
	                              std::error_code& ec) const {
		ec = make_error_code(errc::bad_syntax);
		v1::report header{};
		if (!in.load(header)) return {};
		auto const entry_size = header.builds.size * sizeof(uint32_t);
		if (entry_size < sizeof(v1::report::entry) ||
		    header.strings.offset < sizeof(v1::files) / sizeof(uint32_t) ||
		    header.builds.offset < header.strings.offset)
			return {};

		if (!in.skip((header.strings.offset * sizeof(uint32_t)) -
		             sizeof(header)))
			return {};
		strings_view strings{};
		if (!strings.load_from(in, header.strings)) return {};

		if (!strings.is_valid(header.git.branch) ||
		    !strings.is_valid(header.git.author.name) ||
		    !strings.is_valid(header.git.author.email) ||
		    !strings.is_valid(header.git.committer.name) ||
		    !strings.is_valid(header.git.committer.email) ||
		    !strings.is_valid(header.git.message))
			return {};

		auto const at = [&](io::str offset) { return strings.at(offset); };

		auto branch = at(header.git.branch),
		     author_name = at(header.git.author.name),
		     author_email = at(header.git.author.email),
		     committer_name = at(header.git.committer.name),
		     committer_email = at(header.git.committer.email),
		     message = at(header.git.message);

		if (!in.skip((header.builds.offset -
		              (header.strings.offset + header.strings.size)) *
		             sizeof(uint32_t)))
			return {};

		std::vector<std::unique_ptr<cov::report::build>> builds{};
		if (!load_resources(builds, in, strings,
		                    static_cast<uint32_t>(entry_size),
		                    static_cast<uint32_t>(header.builds.count)))
			return {};

		ec.clear();
		return cov::report::create(
		    id, header.parent, header.file_list, header.git.commit_id, branch,
		    {author_name, author_email}, {committer_name, committer_email},
		    message, header.git.committed.to_seconds(),
		    header.added.to_seconds(), header.stats, std::move(builds));
	}

	bool report::store(ref_ptr<counted> const& value, write_stream& out) const {
		auto const obj =
		    as_a<cov::report>(static_cast<object const*>(value.get()));

		auto stg = [obj] {
			strings_builder strings{};
			strings.insert(obj->branch());
			strings.insert(obj->author_name());
			strings.insert(obj->author_email());
			strings.insert(obj->committer_name());
			strings.insert(obj->committer_email());
			strings.insert(obj->message());

			for (auto const& entry : obj->entries()) {
				strings.insert(entry->props_json());
			}

			return strings.build();
		}();

		auto const locate = [&, size = stg.size()](std::string_view value) {
			auto const offset = stg.locate_or(value, size + 1);
			auto const offset32 = static_cast<uint32_t>(offset);
			if (offset != offset32) throw false;
			return static_cast<io::str>(offset32);
		};

		auto const time_stamp = [](sys_seconds time) {
			timestamp ts{};
			ts = time;
			return ts;
		};

		v1::report report{
		    .strings = stg.after<v1::report>(),
		    .parent = obj->parent_id().id,
		    .file_list = obj->file_list_id().id,
		    .builds = stg.align_array<v1::report, v1::report::entry>(
		        obj->entries().size()),
		    .added = time_stamp(obj->add_time_utc()),
		    .git = {.branch = locate(obj->branch()),
		            .author = {.name = locate(obj->author_name()),
		                       .email = locate(obj->author_email())},
		            .committer = {.name = locate(obj->committer_name()),
		                          .email = locate(obj->committer_email())},
		            .message = locate(obj->message()),
		            .commit_id = obj->commit_id().id,
		            .committed = time_stamp(obj->commit_time_utc())},
		    .stats = obj->stats(),
		};

		if (!out.store(report)) return false;
		if (!out.store({stg.data(), stg.size()})) return false;

		for (auto const& entry : obj->entries()) {
			auto& in = *entry;

			if (!out.store(v1::report::entry{
			        .build = in.build_id().id,
			        .propset = locate(in.props_json()),
			        .stats = in.stats(),
			    }))
				return false;
		}

		return true;
	}
}  // namespace cov::io::handlers

namespace cov {
	report::build::~build() {}

	ref_ptr<report> report::create(
	    git::oid_view oid,
	    git::oid_view parent,
	    git::oid_view file_list,
	    git::oid_view commit,
	    std::string_view branch,
	    signature_view author,
	    signature_view committer,
	    std::string_view message,
	    sys_seconds commit_time_utc,
	    sys_seconds add_time_utc,
	    io::v1::coverage_stats const& stats,
	    std::vector<std::unique_ptr<build>>&& builds) {
		return make_ref<io::handlers::impl>(
		    oid, parent, file_list, commit, branch, author, committer, message,
		    commit_time_utc, add_time_utc, stats, std::move(builds));
	}

	report::builder& report::builder::add(std::unique_ptr<build>&& entry) {
		auto const props = normalize(entry->props_json());
		entries_[props] = std::move(entry);
		return *this;
	}

	report::builder& report::builder::add(git::oid_view build_id,
	                                      std::string_view props,
	                                      io::v1::coverage_stats stats) {
		return add(
		    std::make_unique<io::handlers::build_impl>(build_id, props, stats));
	}

	bool report::builder::remove(std::string_view props_json) {
		auto const props = normalize(props_json);
		auto it = entries_.find(props);
		if (it == entries_.end()) return false;
		entries_.erase(it);
		return true;
	}

	std::vector<std::unique_ptr<report::build>> report::builder::release() {
		std::vector<std::unique_ptr<report::build>> entries{};
		entries.reserve(entries_.size());
		std::transform(std::move_iterator(entries_.begin()),
		               std::move_iterator(entries_.end()),
		               std::back_inserter(entries),
		               [](auto&& pair) { return std::move(pair.second); });
		entries_.clear();
		return entries;
	}  // GCOV_EXCL_LINE[GCC]

	std::string report::builder::normalize(std::string_view props) {
		// TODO: got through a JSON parser
		return {props.data(), props.size()};
	}
}  // namespace cov
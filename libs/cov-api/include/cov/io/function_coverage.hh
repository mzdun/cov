// Copyright (c) 2022 Marcin Zdun
// This code is licensed under MIT license (see LICENSE for details)

#pragma once

#include <cov/io/db_object.hh>
#include <cov/report.hh>

namespace cov::io::handlers {
	struct function_coverage : db_handler_for<cov::function_coverage> {
		ref_ptr<counted> load(uint32_t magic,
		                      uint32_t version,
		                      git::oid_view id,
		                      read_stream& in,
		                      std::error_code& ec) const override;
		bool store(ref_ptr<counted> const& obj,
		           write_stream& out) const override;
	};
}  // namespace cov::io::handlers

// THIS FILE IS AUTOGENERATED
// @generated
#pragma once

#include <lngs/lngs.hpp>

// clang-format off
namespace cov::app::str::cov_init {
    enum class lng {
        /// points to connected Git repository; defaults to current repo (Description for the --git-dir argument)
        ARG_GITDIR = 1001,
        /// will re-init the repository with a fresh database (Description for the --force argument)
        ARG_FORCE = 1002,
        /// sets the directory to put the repository in; defaults to current directory (Description for the init's last argument)
        ARG_DIRECTORY = 1003,
        /// directory (Name of the init's last argument; note this one does not have angle brackets)
        DIR_META = 1004,
        /// not git repo: {0} (Error message for wrong initial directory)
        NOT_GIT = 1005,
        /// Cov repository already exists in {0} (Error message for pre-exisiting repository)
        EXISTS = 1006,
        /// Cannot remove Cov repository in {0} (Error message for removal of existing repository)
        CANNOT_REMOVE = 1007,
        /// Cannot initialize Cov repository in {0} (Error message for filesystem issues with new repository)
        CANNOT_INITIALIZE = 1008,
        /// Initialized empty Cov repository in {0} (Message for successful initialization)
        INITIALIZED = 1009,
        /// Reinitialized empty Cov repository in {0} (Message for successful re-initialization)
        REINITIALIZED = 1010,
        /// Using Git repository in {0} (Message pointing to dependent git repository)
        USING_GIT = 1011,
    }; // enum class lng

    struct Resource {
        static const char* data();
        static std::size_t size();
    };

    using Strings = lngs::SingularStrings<lng, lngs::VersionedFile<1,
        lngs::storage::FileWithBuiltin<Resource>>>;
} // namespace cov::app::str::cov_init
// clang-format on

// THIS FILE IS AUTOGENERATED
// @generated
#pragma once

#include <lngs/lngs.hpp>

// clang-format off
namespace cov::app::str::cov_config {
    enum class lng {
        /// <file-options> (Name of a file options argument(s))
        SCOPE_META = 1001,
        /// file options (Header for list of scopes)
        SCOPE_TITLE = 1002,
        /// <no option> (Name for the situation, when neither local, global, nor system scopes were chosen.)
        NO_SCOPE_META = 1003,
        /// reads from all visible scopes at once, writes to .covdata/config (Description for missing scope)
        NO_SCOPE_DESCRIPTION = 1004,
        /// chooses .covdata/config (Description for the --local scope)
        SCOPE_LOCAL = 1005,
        /// chooses ~/.config/cov/config (Description for the --global scope)
        SCOPE_GLOBAL = 1006,
        /// chooses $(prefix)/etc/covconfig (Description for the --system scope)
        SCOPE_SYSTEM = 1007,
        /// chooses given file (Description for the --file scope)
        SCOPE_FILE = 1008,
        /// when only name is given, reads its value; otherwise sets the new value (Description for the positional arguments (name and optional value))
        NAME_VALUE = 1009,
        /// adds a new line to the option without altering any existing values (Description for the --add scope)
        ADD = 1010,
        /// prints the value matching the key name (Description for the --get scope)
        GET = 1011,
        /// prints all values matching the key name (Description for the --get-all scope)
        GET_ALL = 1012,
        /// removes the value matching the key name if there is exactly one (Description for the --unset scope)
        UNSET = 1013,
        /// removes all values matching the key name (Description for the --unset-all scope)
        UNSET_ALL = 1014,
        /// lists all variables in a config files (Description for the --list scope)
        LIST_ENTRIES = 1015,
        /// too many arguments (Error message for name/value pair, which seems to have a triplet)
        TOO_MANY_ARGUMENTS = 1016,
        /// needs both <name> and <value> (Error message for --add with only one option)
        ADD_VALUE_MISSING = 1017,
    }; // enum class lng

    struct Resource {
        static const char* data();
        static std::size_t size();
    };

    using Strings = lngs::SingularStrings<lng, lngs::VersionedFile<1,
        lngs::storage::FileWithBuiltin<Resource>>>;
} // namespace cov::app::str::cov_config
// clang-format on

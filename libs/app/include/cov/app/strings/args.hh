// THIS FILE IS AUTOGENERATED
#pragma once

#include <lngs/lngs.hpp>

// clang-format off
namespace cov::app::str::args {
    enum class lng {
        /// usage:  (Synopsis header; please note there is one space at the end of this entry)
        USAGE = 1001,
        /// <arg> (Default name of an argument)
        DEF_META = 1002,
        /// positional arguments (Header for list of positional arguments)
        POSITIONALS = 1003,
        /// optional arguments (Header for list of optional arguments)
        OPTIONALS = 1004,
        /// shows this help message and exits (Description for the -h/--help argument)
        HELP_DESCRIPTION = 1005,
        /// unrecognized argument: {0} (Error message for an unrecognized argument; the placeholder will contain the name of the argument)
        UNRECOGNIZED = 1006,
        /// argument {0}: expected one argument (Error message for a missing argument)
        NEEDS_PARAM = 1007,
        /// argument {0}: value was not expected (Error message for unneeded value)
        NEEDS_NO_PARAM = 1008,
        /// argument {0}: expected a number (Error message for a numeric argument, when parsing failed)
        NEEDS_NUMBER = 1009,
        /// argument {0}: number outside of expected bounds (Error message for a numeric argument, when parsing would get outside of representable bounds)
        NEEDED_NUMBER_EXCEEDED = 1010,
        /// argument {0}: value {1} is not recognized (Error message for a list of arguments, when parsing failed)
        NEEDS_ENUM_UNKNOWN = 1011,
        /// known values for {0}: {1} (Help message presenting a list of possible values for a list-type argument)
        NEEDS_ENUM_KNOWN_VALUES = 1012,
        /// argument {0} is required (Error message for a missing required argument)
        REQUIRED = 1013,
        /// {0}: error: {1} (Error message template; placeholder 0 will get the name of program, placeholder 1 an actual message)
        ERROR_MSG = 1014,
    }; // enum class lng

    struct Resource {
        static const char* data();
        static std::size_t size();
    };

    using Strings = lngs::SingularStrings<lng, lngs::VersionedFile<1,
        lngs::storage::FileWithBuiltin<Resource>>>;
} // namespace cov::app::str::args
// clang-format on

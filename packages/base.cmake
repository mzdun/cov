if("x${CMAKE_SYSTEM_NAME}" STREQUAL "xCYGWIN")
    set(CMAKE_SYSTEM_NAME cygwin)
else()
    string(TOLOWER ${CMAKE_SYSTEM_NAME} CMAKE_SYSTEM_NAME_LOWER)
    if(${CMAKE_SYSTEM_NAME_LOWER} MATCHES linux)
        find_program(LSB_RELEASE_EXEC lsb_release)
        execute_process(COMMAND ${LSB_RELEASE_EXEC} -is
            OUTPUT_VARIABLE LSB_RELEASE_ID_SHORT
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        execute_process(COMMAND ${LSB_RELEASE_EXEC} -rs
            OUTPUT_VARIABLE LSB_RELEASE_REL_SHORT
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        string(TOLOWER ${LSB_RELEASE_ID_SHORT} LSB_RELEASE_ID_SHORT_LOWER)
        if (${LSB_RELEASE_ID_SHORT_LOWER} MATCHES ubuntu)
            set(CMAKE_SYSTEM_NAME_LOWER "${LSB_RELEASE_ID_SHORT_LOWER}-${LSB_RELEASE_REL_SHORT}")
        endif()
    endif()
    set(CMAKE_SYSTEM_NAME ${CMAKE_SYSTEM_NAME_LOWER}-${CMAKE_SYSTEM_PROCESSOR})
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES windows)
    if(CMAKE_CL_64)
        set(CMAKE_SYSTEM_NAME windows-x86_64)
    else()
        set(CMAKE_SYSTEM_NAME windows-x86_32)
    endif()
endif()

set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VENDOR "midnightBITS")
set(CPACK_PACKAGE_CONTACT "Marcin Zdun <mzdun@midnightbits.com>")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_DESCRIPTION "${PROJECT_DESCRIPTION}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "Cov")
set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "midnightBITS/Cov")
set(CPACK_PACKAGE_ICON "${PROJECT_SOURCE_DIR}/data/assets/appicon-256.png")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}${PROJECT_VERSION_STABILITY}-${CMAKE_SYSTEM_NAME}")
# set(CPACK_PACKAGE_CHECKSUM SHA256)
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND NOT APPLE)
    string(REPLACE "." ";" _COMPILER_CHUNKS ${CMAKE_CXX_COMPILER_VERSION})
    list(GET _COMPILER_CHUNKS 0 CMAKE_CXX_COMPILER_VERSION_MAJOR)
    set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_FILE_NAME}-clang${CMAKE_CXX_COMPILER_VERSION_MAJOR}")
endif()
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_FILE_NAME}-dbg")
endif()
set(CPACK_PACKAGE_DIRECTORY "${PROJECT_BINARY_DIR}/packages")
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_BINARY_DIR}/packages/_CPack_Packages/LICENSE.txt")
file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/packages/_CPack_Packages")
file(COPY_FILE "${PROJECT_SOURCE_DIR}/LICENSE" "${PROJECT_BINARY_DIR}/packages/_CPack_Packages/LICENSE.txt")
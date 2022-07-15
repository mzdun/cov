
set(BUILTINS
    init
)

set(SOURCES
	cov/cov.cc
)

foreach(NAME ${BUILTINS})
    string(REPLACE "-" "_" SAFE_NAME "${NAME}")
    list(APPEND SOURCES builtins/builtin_${SAFE_NAME}.cc)
endforeach()

string(REPLACE ";" ", " BUILTIN_LIST "${BUILTINS}")
message(STATUS "Builtins: ${BUILTIN_LIST}")
include(builtins.cmake)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
source_group(TREE ${CMAKE_CURRENT_BINARY_DIR} FILES ${CMAKE_CURRENT_BINARY_DIR}/cov/builtins.hh)

list(APPEND SOURCES ${CMAKE_CURRENT_BINARY_DIR}/cov/builtins.hh)

add_executable(cov ${SOURCES})
target_compile_options(cov PRIVATE ${ADDITIONAL_WALL_FLAGS})
target_link_options(cov PRIVATE ${ADDITIONAL_LINK_FLAGS})
target_include_directories(cov PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)
target_link_libraries(cov PRIVATE cov-rt)
set_target_properties(cov PROPERTIES FOLDER apps)

install(TARGETS cov
    RUNTIME DESTINATION ${BINARY_DIR}
    COMPONENT main_exec
)

function(setup_cov_core COV_TOOL)
	set_target_properties(cov-${COV_TOOL} PROPERTIES FOLDER apps/core)
	target_link_libraries(cov-${COV_TOOL} PRIVATE app)
    target_link_options(cov-${COV_TOOL} PRIVATE ${ADDITIONAL_LINK_FLAGS})
	target_compile_options(cov-${COV_TOOL} PRIVATE ${ADDITIONAL_WALL_FLAGS})

	set_target_properties(cov-${COV_TOOL} PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CORE_DIR}"
    )

	foreach(BUILD_TYPE DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
		set_target_properties(cov-${COV_TOOL} PROPERTIES
			RUNTIME_OUTPUT_DIRECTORY_${BUILD_TYPE} "${CMAKE_BINARY_DIR}/${CORE_DIR}"
        )
	endforeach()
endfunction()

function(add_cov_core COV_TOOL)
	add_executable(cov-${COV_TOOL} cov-${COV_TOOL}.cc)
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES cov-${COV_TOOL}.cc)

	setup_cov_core(${COV_TOOL})
	install(TARGETS cov-${COV_TOOL}
		RUNTIME DESTINATION ${CORE_DIR}
        COMPONENT tools
    )
endfunction()

if (COV_TESTING)
    enable_testing()

    add_executable(cov-echo tests/cov-echo.cc)
    target_compile_options(cov-echo PRIVATE ${ADDITIONAL_WALL_FLAGS})
    target_link_options(cov-echo PRIVATE ${ADDITIONAL_LINK_FLAGS})
    target_link_libraries(cov-echo PRIVATE fmt::fmt mbits::args)
    set_target_properties(cov-echo PROPERTIES FOLDER tests/spawn)

	set_target_properties(cov-echo PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/elsewhere/${CORE_DIR}"
    )

	foreach(BUILD_TYPE DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
		set_target_properties(cov-echo PROPERTIES
			RUNTIME_OUTPUT_DIRECTORY_${BUILD_TYPE} "${CMAKE_BINARY_DIR}/elsewhere/${CORE_DIR}"
        )
	endforeach()

    add_test(
        NAME cov-exec
        COMMAND "${Python3_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/exe_driver.py"
            --target "$<TARGET_FILE:cov>"
            --tests "${CMAKE_CURRENT_SOURCE_DIR}/tests/main-set"
            --data-dir "${CMAKE_CURRENT_SOURCE_DIR}/tests/data"
            --version "${PROJECT_VERSION}${PROJECT_VERSION_STABILITY}${PROJECT_VERSION_BUILD_META}"
            --install "${CMAKE_CURRENT_SOURCE_DIR}/tests/copy"
            --install-with "$<TARGET_FILE:cov-echo>"
            )
endif()

cpack_add_component(main_exec
  DISPLAY_NAME "Main executable"
  GROUP apps
)

cpack_add_component(tools
  DISPLAY_NAME "Core tools"
  GROUP apps
)

cpack_add_component_group(apps
  DISPLAY_NAME "Executables"
  EXPANDED
)

if (CMAKE_GENERATOR MATCHES "Visual Studio" AND TARGET cov_coveralls_test)
  add_dependencies(cov_coveralls_test
    cov-echo
  )
endif()
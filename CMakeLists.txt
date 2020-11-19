cmake_minimum_required(VERSION 3.13..3.16 FATAL_ERROR)

set(PROJECT_NAME abc)
project(${PROJECT_NAME}
    VERSION 0.1.0
    LANGUAGES CXX
)

option(ENABLE_UNIT_TESTS "Enable unit tests" ON)

#Make sure that custom modules like FindRapidJSON are found
list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_SOURCE_DIR}/cmake)

###############################################################################
# Dependencies

set(MIN_doctest_VERSION 1.1)
find_package(doctest ${MIN_doctest_VERSION} REQUIRED MODULE)

###############################################################################
# Targets and properties

add_library(${PROJECT_NAME}
    src/core.cpp
    src/debug.cpp
    src/enum.cpp
    #src/memory_mapped_file.cpp
    src/pointer.cpp
    #
    include/abc/algo.hpp
    include/abc/chrono.hpp
    include/abc/core.hpp
    include/abc/crash.hpp
    include/abc/debug.hpp
    include/abc/enum.hpp
    include/abc/format.hpp
    include/abc/format_chrono.hpp
    include/abc/formatters.hpp
    include/abc/function.hpp
    include/abc/memory_mapped_file.hpp
    include/abc/optional.hpp
    include/abc/platform/platform.hpp
    include/abc/pointer.hpp
    include/abc/profiler.hpp
    include/abc/result.hpp
    include/abc/string.hpp
    include/abc/tagged_type.hpp
    include/abc/timer.hpp
    include/abc/utils.hpp
)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
    PUBLIC 
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_subdirectory(src)

###############################################################################
# Set properties

target_compile_definitions(${PROJECT_NAME} INTERFACE $<$<CXX_COMPILER_ID:MSVC>:NOMINMAX>)
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
            -Wall -Wextra -Wpedantic
            #-Werror
        #blacklisted
            -Wno-c++98-compat
            -Wno-c++98-compat-pedantic
            -Wno-duplicate-enum
            -Wno-exit-time-destructors
            -Wno-gnu-zero-variadic-macro-arguments
            -Wno-zero-as-null-pointer-constant
            -Wno-unknown-warning
>
    $<$<CXX_COMPILER_ID:MSVC>:/WX>
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_11)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
    PRIVATE
)

###############################################################################
# Installation

include(GNUInstallDirs)
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

install(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

#This is required so that the exported target has a formatted name
#set_target_properties(${PROJECT_NAME} PROPERTIES EXPORT_NAME AbcLib)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT ${PROJECT_NAME}-targets
  FILE
    ${PROJECT_NAME}Targets.cmake
  NAMESPACE
    ${PROJECT_NAME}::
  DESTINATION
    ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

#Create a ConfigVersion.cmake file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(${CMAKE_CURRENT_LIST_DIR}/cmake/${PROJECT_NAME}Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    INSTALL_DESTINATION ${INSTALL_CONFIGDIR}
)

#Install the config, configversion and custom find modules
install(FILES
    ${CMAKE_CURRENT_LIST_DIR}/cmake/Finddoctest.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
    DESTINATION ${INSTALL_CONFIGDIR}
)

##############################################
## Exporting from the build tree
configure_file(${CMAKE_CURRENT_LIST_DIR}/cmake/Finddoctest.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/Finddoctest.cmake
    COPYONLY)

export(EXPORT ${PROJECT_NAME}-targets
    FILE ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake
    NAMESPACE ${PROJECT_NAME}::)

#Register package in user's package registry
export(PACKAGE ${PROJECT_NAME})

##############################################
## Add tests

if(${ENABLE_UNIT_TESTS})
    enable_testing()
    add_subdirectory(tests)
endif()

# Report summary of packages found
include(FeatureSummary)
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    feature_summary(WHAT ENABLED_FEATURES DISABLED_FEATURES PACKAGES_FOUND)
    feature_summary(FILENAME ${CMAKE_CURRENT_BINARY_DIR}/features.log WHAT ALL)
endif()

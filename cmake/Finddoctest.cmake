# FindDocTest.cmake
#
# Finds the doctest library
#
# This will define the following variables
#
#    doctest_FOUND
#    doctest_INCLUDE_DIRS
#
# and the following imported targets
#
#     doctest::doctest
#
# Author: Pablo Arias - pabloariasal@gmail.com

find_package(PkgConfig)
pkg_check_modules(PC_doctest QUIET doctest)

find_path(doctest_INCLUDE_DIR
    NAMES doctest/doctest.h
    PATHS
		${PC_doctest_INCLUDE_DIRS}
	    ${CMAKE_SOURCE_DIR}/extern/doctest/
    PATH_SUFFIXES doctest
	DOC "The directory where doctest/doctest.h resides"
)

set(doctest_VERSION ${PC_doctest_VERSION})

mark_as_advanced(doctest_FOUND doctest_INCLUDE_DIR doctest_VERSION)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(doctest
    REQUIRED_VARS doctest_INCLUDE_DIR
    VERSION_VAR doctest_VERSION
)

if(doctest_FOUND)
    set(doctest_INCLUDE_DIRS ${doctest_INCLUDE_DIR})
endif()

if(doctest_FOUND AND NOT TARGET doctest::doctest)
    add_library(doctest::doctest INTERFACE IMPORTED)
    set_target_properties(doctest::doctest PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${doctest_INCLUDE_DIR}"
    )
endif()

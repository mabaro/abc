include(FindPackageHandleStandardArgs)

find_path(
	doctest_INCLUDE_DIR
	NAMES doctest/doctest.h
	PATHS
    ${CMAKE_SOURCE_DIR}/extern/doctest/include/
	${doctest_ROOT_DIR}/include
	DOC "The directory where doctest/doctest.h resides")

# Handle REQUIRED argument, define *_FOUND variable
find_package_handle_standard_args(doctest DEFAULT_MSG doctest_INCLUDE_DIR)

if (doctest_FOUND)
	set(doctest_INCLUDE_DIRS ${doctest_INCLUDE_DIR})
endif()

# Hide some variables
mark_as_advanced(doctest_INCLUDE_DIR)

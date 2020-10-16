include(FindPackageHandleStandardArgs)

find_path(
	GLM_INCLUDE_DIR
	NAMES glm/glm.hpp
	PATHS
    ${CMAKE_SOURCE_DIR}/extern/glm/
	${GLM_ROOT_DIR}/include
	DOC "The directory where glm/glm.hpp resides")

# Handle REQUIRED argument, define *_FOUND variable
find_package_handle_standard_args(GLM DEFAULT_MSG GLM_INCLUDE_DIR)

# Define GLM_INCLUDE_DIRS
if (GLM_FOUND)
	set(GLM_INCLUDE_DIRS ${GLM_INCLUDE_DIR})
endif()

# Hide some variables
mark_as_advanced(GLM_INCLUDE_DIR)

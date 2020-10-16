### Require out-of-source builds
file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" LOC_PATH)
if(EXISTS "${LOC_PATH}")
    message(FATAL_ERROR "You cannot build in a source directory (or any directory with a CMakeLists.txt file). Please make a build subdirectory. Feel free to remove CMakeCache.txt and CMakeFiles.")
endif()

### Restrict build types and set default build type to RelWithDebInfo if none was specified
set(allowedBuildTypes Debug Release RelWithDebInfo MinSizeRel)
get_property(isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG) # CMAKE > 3.9
if (isMultiConfig)
    if (NOT PATHTRACER_DEFAULT_BUILD_TYPE IN_LIST CMAKE_CONFIGURATION_TYPES)
        if (PATHTRACER_DEFAULT_BUILD_TYPE IN_LIST allowedBuildTypes)
            list(APPEND CMAKE_CONFIGURATION_TYPES ${PATHTRACER_DEFAULT_BUILD_TYPE})
        endif()
    endif()
else()
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS ${allowedBuildTypes})

    if (NOT CMAKE_BUILD_TYPE)
        message(STATUS "No CMAKE_BUILD_TYPE selected. Setting default value to ${PATHTRACER_DEFAULT_BUILD_TYPE}")
        set(CMAKE_BUILD_TYPE ${PATHTRACER_DEFAULT_BUILD_TYPE} CACHE STRING "Choose the type of build" FORCE)
    elseif(NOT CMAKE_BUILD_TYPE IN_LIST allowedBuildTypes)
        message(FATAL_ERROR "Invalid build type: ${CMAKE_BUILD_TYPE}. Only these are allowed: ${allowedBuildTypes}")
    endif()
endif()

# Generate compile_commands.json to make it easier to work with clang based
# tools
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ENABLE_IPO "Enable Interprocedural Optimization, aka Link Time Optimization (LTO)" OFF)

if(ENABLE_IPO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT result OUTPUT output)
    if (result)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    else()
        message(WARNING "IPO is not supported: ${output}")
    endif()
endif()


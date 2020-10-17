cmake_minimum_required(VERSION 3.17)

if(${CMAKE_VERSION} VERSION_LESS 3.10)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

project(cpp
    VERSION 1.0
    DESCRIPTION "CPP"
    LANGUAGES CXX C
)

###################################################################

option(ENABLE_UNIT_TESTS "Enable unit tests" ON)
option(ENABLE_UNITY_BUILD "Enable Unity build" OFF)
option(CMAKE_BUILD_TYPE "Build type in Release,Debug" Release)

set(CMAKE_VERBOSE_MAKEFILE ON)

###################################################################

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME) # main CmakeLists.txt
    # Optionally set things like CMAKE_CXX_STANDARD, CMAKE_POSITION_INDEPENDENT_CODE here
    
    # Let's ensure -std=c++xx instead of -std=g++xx
    set(CMAKE_CXX_EXTENSIONS OFF)

    # Let's nicely support folders in IDE's
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)

    # Docs only available if this is the main app
    find_package(Doxygen)
    if(Doxygen_FOUND)
        add_subdirectory(docs)
    else()
        message(STATUS "Doxygen not found, not building docs")
    endif()

	###################################################################

    if(ENABLE_UNIT_TESTS)
        # setup testing framework
        #find_package(doctest REQUIRED)
    
        set(DOCTEST_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern/doctest)
        add_library(doctest::doctest IMPORTED INTERFACE)
        set_target_properties(doctest::doctest
            PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES "${DOCTEST_INCLUDE_DIR}"
        )
        
        # enable test framework
        include(CTest) #enable_testing()
    endif()
endif()

#################################################################################

if(ENABLE_UNITY_BUILD)
    #https://onqtam.com/programming/2019-12-20-pch-unity-cmake-3-16/
    #set_target_properties(<target> PROPERTIES UNITY_BUILD ON)
    set(CMAKE_UNITY_BUILD ON) # globally
    #-DCMAKE_UNITY_BUILD=ON   # globally
endif()

#################################################################################

### set cmake extensions directory
#set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake
)

### include useful modules
include(StandardProjectSettings)
include(StaticAnalyzers)
include(CompilerCache)

# Link this 'library' to set the c++ standard / compile-time options requested
add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE)
include(CompilerOptions) # exceptions, rtti, pch

# Link this 'library' to use the warnings specified in CompilerWarnings.cmake
add_library(project_warnings INTERFACE)
## standard compiler warnings
include(CompilerWarnings)
target_set_compile_warnings(project_warnings)

#################################################################################
# actual project definition starts here

FILE(GLOB_RECURSE headers include/abc/*.hpp)
FILE(GLOB_RECURSE sources src/*.cpp src/*.hpp include/abc/*.hpp)

add_library(abc STATIC)
target_sources(abc
	PRIVATE ${sources}
	PUBLIC
)
target_include_directories(abc
	PUBLIC include
	PRIVATE
)
target_link_libraries(abc
	PUBLIC 
	PRIVATE project_options project_warnings
)

if(ENABLE_UNIT_TESTS AND CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    message(STATUS "* Enabled UNIT_TESTS")
    add_compile_definitions(ENABLE_UNIT_TESTS)

    target_link_libraries(project_options INTERFACE doctest::doctest)
    
    add_subdirectory(tests)
endif()

# Report summary of packages found
include(FeatureSummary)
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    feature_summary(WHAT ENABLED_FEATURES DISABLED_FEATURES PACKAGES_FOUND)
    feature_summary(FILENAME ${CMAKE_CURRENT_BINARY_DIR}/features.log WHAT ALL)
endif()

#install(TARGETS abc DESTINATION lib)
#install(TARGETS ${headers} DESTINATION include)

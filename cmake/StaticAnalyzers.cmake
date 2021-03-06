option(ENABLE_CPPCHECK             "Enable static analysis with cppcheck" OFF)
option(ENABLE_CLANG_TIDY           "Enable static analysis with clang-tidy" OFF)
option(ENABLE_INCLUDE_WHAT_YOU_USE "Enable static analysis with include-what-you-use" OFF)
option(ENABLE_CPPLINT              "Enable static analysis with cpplint" OFF)

# https://cmake.org/cmake/help/latest/prop_tgt/LANG_CPPCHECK.html
if(ENABLE_CPPCHECK)
    find_program(CPPCHECK_PROGRAM NAMES cppcheck)
    list(APPEND COMMON_CPPCHECK_FLAGS "--suppress=missingInclude" "--enable=all" "--std=c++11" "--inconclusive" "--force" "--inline-suppr")
    if (CPPCHECK_PROGRAM AND NOT DEFINED ${CMAKE_C_CPPCHECK})
        set(CMAKE_C_CPPCHECK "${CPPCHECK_PROGRAM};${COMMON_CPPCHECK_FLAGS}")
    endif()
    if (CPPCHECK_PROGRAM AND NOT DEFINED ${CMAKE_CXX_CPPCHECK})
        set(CMAKE_CXX_CPPCHECK "${CPPCHECK_PROGRAM};${COMMON_CPPCHECK_FLAGS}")
    endif()
    if (CPPCHECK_PROGRAM)
        message(STATUS "cppcheck found, setting static checks.")
        message(STATUS "CPPCHECK_PROGRAM:   ${CPPCHECK_PROGRAM}")
        message(STATUS "CMAKE_C_CPPCHECK:   ${CMAKE_C_CPPCHECK}")
        message(STATUS "CMAKE_CXX_CPPCHECK: ${CMAKE_CXX_CPPCHECK}")
    else()
        message(WARNING "cppcheck not found, unsetting all CMAKE_<LANG>_CPPCHECK variables")
        unset(CMAKE_C_CPPCHECK)
        unset(CMAKE_CXX_CPPCHECK)
    endif()
endif()

# https://cmake.org/cmake/help/latest/prop_tgt/LANG_CLANG_TIDY.html
if(ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_PROGRAM NAMES clang-tidy)
    set(COMMON_CLANG_TIDY_FLAGS "-checks=-*,readability-*,portability-*,performance-*,android-*,cppcoreguidelines-*,clang-analyzer-*,modernize-*" "-extra-arg=-Wno-unknown-warning-option")
    if (CLANG_TIDY_PROGRAM AND NOT DEFINED ${CMAKE_C_CLANG_TIDY})
        set(CMAKE_C_CLANG_TIDY "${CLANG_TIDY_PROGRAM};${COMMON_CLANG_TIDY_FLAGS}")
    endif()
    if (CLANG_TIDY_PROGRAM AND NOT DEFINED ${CMAKE_CXX_CLANG_TIDY})
        set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_PROGRAM};${COMMON_CLANG_TIDY_FLAGS}")
    endif()
    if (CLANG_TIDY)
        message(STATUS "clang-tidy found, setting static checks.")
        message(STATUS "CLANG_TIDY_PROGRAM:   ${CLANG_TIDY_PROGRAM}")
        message(STATUS "CMAKE_C_CLANG_TIDY:   ${CMAKE_C_CLANG_TIDY}")
        message(STATUS "CMAKE_CXX_CLANG_TIDY: ${CMAKE_CXX_CLANG_TIDY}")
    else()
        message(WARNING "clang-tidy not found, unsetting all CMAKE_<LANG>_CLANG_TIDY variables")
        unset(CMAKE_C_CLANG_TIDY)
        unset(CMAKE_CXX_CLANG_TIDY)
    endif()
endif()

# https://cmake.org/cmake/help/latest/prop_tgt/LANG_INCLUDE_WHAT_YOU_USE.html
if(ENABLE_INCLUDE_WHAT_YOU_USE)
    find_program(INCLUDE_WHAT_YOU_USE_PROGRAM include-what-you-use)
    set(COMMON_INCLUDE_WHAT_YOU_USE_FLAGS "")
    if (INCLUDE_WHAT_YOU_USE_PROGRAM AND NOT DEFINED ${CMAKE_C_INCLUDE_WHAT_YOU_USE})
        set(CMAKE_C_INCLUDE_WHAT_YOU_USE "${INCLUDE_WHAT_YOU_USE_PROGRAM};${COMMON_INCLUDE_WHAT_YOU_USE_FLAGS}")
    endif()
    if (INCLUDE_WHAT_YOU_USE_PROGRAM AND NOT DEFINED ${CMAKE_CXX_INCLUDE_WHAT_YOU_USE})
        set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE "${INCLUDE_WHAT_YOU_USE_PROGRAM};${COMMON_INCLUDE_WHAT_YOU_USE_FLAGS}")
    endif()
    if (INCLUDE_WHAT_YOU_USE_PROGRAM)
        message(STATUS "include-what-you-use found, setting static checks.")
        message(STATUS "INCLUDE_WHAT_YOU_USE_PROGRAM:   ${INCLUDE_WHAT_YOU_USE_PROGRAM}")
        message(STATUS "CMAKE_C_INCLUDE_WHAT_YOU_USE:   ${CMAKE_C_INCLUDE_WHAT_YOU_USE}")
        message(STATUS "CMAKE_CXX_INCLUDE_WHAT_YOU_USE: ${CMAKE_CXX_INCLUDE_WHAT_YOU_USE}")
    else()
        message(WARNING "include-what-you-use not found, unsetting CMAKE_<LANG>_INCLUDE_WHAT_YOU_USE variables")
        unset(CMAKE_C_INCLUDE_WHAT_YOU_USE)
        unset(CMAKE_CXX_INCLUDE_WHAT_YOU_USE)
    endif()
endif()

# https://cmake.org/cmake/help/latest/prop_tgt/LANG_CPPLINT.html
if (ENABLE_CPPLINT)
    find_program(CPPLINT_PROGRAM NAMES cpplint)
    list(APPEND COMMON_CPPLINT_FLAGS "")
    if (CPPLINT_PROGRAM AND NOT DEFINED ${CMAKE_C_CPPLINT})
        set(CMAKE_C_CPPLINT "${CPPLINT_PROGRAM};${COMMON_CPPLINT_FLAGS}")
    endif()
    if (CPPLINT_PROGRAM AND NOT DEFINED ${CMAKE_CXX_CPPLINT})
        set(CMAKE_CXX_CPPLINT "${CPPLINT_PROGRAM};${COMMON_CPPLINT_FLAGS}")
    endif()
    if (CPPLINT_PROGRAM)
        message(STATUS "cpplint found, setting static checks.")
        message(STATUS "CPPLINT_PROGRAM:   ${CPPLINT_PROGRAM}")
        message(STATUS "CMAKE_C_CPPLINT:   ${CMAKE_C_CPPLINT}")
        message(STATUS "CMAKE_CXX_CPPLINT: ${CMAKE_CXX_CPPLINT}")
    else()
        message(WARNING "cpplint not found, unsetting all CMAKE_<LANG>_CPPLINT variables")
        unset(CMAKE_C_CPPLINT)
        unset(CMAKE_CXX_CPPLINT)
    endif()
endif()
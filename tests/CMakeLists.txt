# Tests need to be added as executable first
add_executable(abc_test 
	main.cpp
	algo.cpp
	enum.cpp
	format.cpp
	#memory_mapping.cpp
	optional.cpp
	pointer.cpp
	profiler.cpp
	result.cpp
	tagged_type.cpp
	utils.cpp
)

target_compile_definitions(abc_test PRIVATE ABC_TESTING)
target_include_directories(abc_test PRIVATE )

# Should be linked to the main library, as well as the testing library
target_link_libraries(abc_test 
		abc
		doctest::doctest
		#project_options project_warnings
)

# If you register a test, then ctest and make test will run it.
# You can also run examples and check the output, as well.
add_test(NAME abc_tests COMMAND abc_test) # Command can be a target

#!/bin/bash

BUILD_DIR="build"

#-DCMAKE_BUILD_TYPE=Debug,Release
BUILD_MODE=Release
DEFINES="-DUSE_IMGUI=false -DUSE_GLFW=true -DUSE_VULKAN=false -DUSE_GL=true"

clean()
{
	echo Cleaning project files...
	rm -rf $BUILD_DIR
}

generate()
{
	echo Generating project files...

	if [ ! -d $BUILD_DIR ]; then
		echo "creating cmake build directory: '"$BUILD_DIR"'"
		mkdir $BUILD_DIR
	fi

	cd $BUILD_DIR
	conan install .. -s build_type=$BUILD_MODE
	cd ..

	#generate project + perform build
	cmake -S . -B $BUILD_DIR $2 $3 $4 -DCMAKE_INSTALL_PREFIX=./_install -DCMAKE_BUILD_TYPE=$BUILD_MODE $DEFINES

	# Configure (i.e., add -GNinja)
	# cmake -S . -B $BUILD_DIR

	# build
	# cmake --build $BUILD_DIR

	# test
	# cmake --build $BUILD_DIR --target test

	# docs
	# cmake --build $BUILD_DIR --target docs

	# using IDE
	# cmake -S . -B xbuild -GXcode
	# cmake --open xbuild
}

build()
{
	echo "Building project in '"$BUILD_DIR"'..."
	cmake --build $BUILD_DIR $2 $3 $4
}

test()
{
	echo Running tests...
	cmake --build $BUILD_DIR --target RUN_TESTS
}

run()
{
	APP=app_run
	if ![ "$2" = "" ]
	then
		APP="$2"
	fi

	echo Running custom target: $APP
	cmake --build %BUILD_DIR% --target $APP --config $BUILD_MODE
}

if [ "$1" = "clean" ]
then 
	clean
elif [ "$1" = "build" ]
then
	build
elif [ "$1" = "generate" ]
then
	generate
elif  [ "$1" = "test" ]
then
	test
elif  [ "$1" = "run" ]
then
	run
elif  [ -d $BUILD_DIR ]
then
	build
else
	generate
fi

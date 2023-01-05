#!/bin/bash

PROJECT_NAME="renderer_example"

WARNINGS="-Wformat=2 
          -Wmain 
          -Wparentheses 
          -Wuninitialized
          -Wsign-compare 
          -Werror"

COMPILER_FLAGS=""
LINKER_FLAGS="-lX11 -pthread -lGL ./dependencies/GLEW/lib/libGLEW.a"
INCLUDE_FOLDERS="-I ../src/
                 -I ./dependencies/
                 -I ./dependencies/GLEW/include/
                 -I ./dependencies/platform/src/
                 -I ./dependencies/cmath/
"

pushd "$(dirname ${BASH_SOURCE[0]})"

if [ ! -d "build" ]
then
    mkdir build
fi

# command line arguments
if [[ $1 == "debug" ]]
then
    echo "debug build"
    COMPILER_FLAGS="-O0 $COMPILER_FLAGS"
    PREPROCESSOR="-g"
else
    echo "release build"
    COMPILER_FLAGS="-O3 $COMPILER_FLAGS"
    PREPROCESSOR="-s"
fi

gcc $PREPROCESSOR $COMPILER_FLAGS $WARNINGS $INCLUDE_FOLDERS "./src/main.c" -lm -o "./build/renderer_example.a" $LINKER_FLAGS

popd

exit 0

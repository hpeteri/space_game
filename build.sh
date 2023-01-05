#!/bin/bash

PROJECT_NAME="renderer_example"

WARNINGS="-Wformat=2 
          -Wmain 
          -Wparentheses 
          -Wuninitialized
          -Wsign-compare 
          -Werror"

COMPILER_FLAGS="-pipe "
LIBS="-lm -no-pie"
LINKER_FLAGS="-lX11 -pthread -lGL ./dependencies/GLEW/lib/libGLEW.a"
INCLUDE_FOLDERS="-I ../src/
                 -I ./dependencies/
                 -I ./dependencies/GLEW/include/
                 -I ./dependencies/platform/src/
                 -I ./dependencies/cmath/
                 -I ./dependencies/sglr/src/
"

pushd "$(dirname ${BASH_SOURCE[0]})"

if [ ! -d "build" ]
then
    mkdir build
fi

# command line arguments
if [[ $1 == "release" ]]
then
    echo "release build"
    COMPILER_FLAGS+="-O2 -D RELEASE_BUILD "
    PREPROCESSOR="-s"
else
    echo "debug build"
    COMPILER_FLAGS+="-O0 -D DEBUG_BUILD "
    PREPROCESSOR="-g"
fi

if [[ $2 == "analyze" ]]
then
    echo "analyze"
    COMPILER_FLAGS+=" -fanalyzer"
fi
    

# === single translation unit build ===

SRC=" ./src/unity_build.c"
time gcc $PREPROCESSOR $COMPILER_FLAGS $WARNINGS $INCLUDE_FOLDERS $SRC $LIBS -o "./build/renderer_example.a" $LINKER_FLAGS

popd
exit 0

# === normal build ===

SRC=""
SRC+="./dependencies/platform/src/n1_platform_main.c "
SRC+="./dependencies/sglr/src/sglr.c                 "

SRC+=" ./src/main.c              "
SRC+="./src/editor/editor_main.c "
SRC+="./src/console/console.c    "
SRC+="./src/input/input.c        "
SRC+="./src/tweak/tweak.c        "

time gcc $PREPROCESSOR $COMPILER_FLAGS $WARNINGS $INCLUDE_FOLDERS $SRC $LIBS -o "./build/renderer_example.a" $LINKER_FLAGS

popd

exit 0

#!/bin/bash 

printf "\t=========== Building MyMillions contract ===========\n\n"

RED='\033[0;31m'
NC='\033[0m'

CORES=`getconf _NPROCESSORS_ONLN`
if [[ -d "build" ]]; then
  rm -rvf ./build&> /dev/null
fi
mkdir -p build
pushd build &> /dev/null
cmake -DCMAKE_BINARY_DIR=./output/ ../../sources/
make -j${CORES}
popd &> /dev/null

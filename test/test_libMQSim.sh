#! /bin/bash

PROJ_HOME=$(pwd)
BUILD="$PROJ_HOME"/build/test

make clean

mkdir -p "$BUILD"

make -j$(nproc) all

cd "$PROJ_HOME"/test

g++ -O0 -g -g3 -ggdb3 -fno-omit-frame-pointer -DLIBMQ_TEST -DSINGLETON -o "$BUILD"/test_libMQSim TestLibMQSim.cpp -I"$PROJ_HOME" -L"$PROJ_HOME" -lMQSim

echo "DONE!!"
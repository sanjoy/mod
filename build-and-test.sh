#!/bin/bash

CXX=${CXX-clang++}
CXXFLAGS=${CXXFLAGS-}

mkdir -p build
${CXX} -Wall -Werror -std=c++11 ${CXXFLAGS} test.cpp -o build/test && ./build/test

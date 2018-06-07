#!/bin/sh
cd build && cmake \
    -DCMAKE_INSTALL_PREFIX=/home/boris/area51/triang/libs \
    -DCMAKE_CXX_FLAGS=-std=c++11 \
    -DCMAKE_BUILD_TYPE=Debug \
    ..

#!/usr/bin/env bash
set -ex

OPENCV_VERSION=4.1.0
pushd ~/opencv/opencv-$OPENCV_VERSION
mkdir -p build
pushd build
cmake -D CMAKE_BUILD_TYPE=RELEASE \
      -D WITH_OPENGL=ON \
      -D CMAKE_INSTALL_PREFIX=/usr/local \
      -D OPENCV_EXTRA_MODULES_PATH=../../opencv_contrib-$OPENCV_VERSION/modules \
      -D OPENCV_ENABLE_NONFREE=ON \
      -D BUILD_PERF_TESTS=OFF \
      -D BUILD_TESTS=OFF \
      -D BUILD_DOCS=ON \
      -D BUILD_EXAMPLES=ON \
      -D WITH_TBB=ON \
      -D WITH_OPENMP=ON \
      ..
make -j "$(nproc)"
popd; popd

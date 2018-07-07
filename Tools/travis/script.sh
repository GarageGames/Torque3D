#!/bin/bash
set -e

if [[ $TRAVIS_OS_NAME = linux ]]; then
    mkdir -p My\ Projects/Torque3D/buildFiles/ubuntu
    pushd My\ Projects/Torque3D/buildFiles/ubuntu
    cmake ../../../.. -DTORQUE_APP_NAME=Torque3D $ADDITIONAL_CMAKE_ARGS
    echo CFLAGS:: $CFLAGS LDFLAGS:: $LDFLAGS
    make
    make install || true
    pushd ../../game
    ./Torque3D runTests.cs || true
    popd
    popd

elif
    [[ $TRAVIS_OS_NAME = osx ]]; then
    mkdir -p My\ Projects/Torque3D/
    pushd My\ Projects/Torque3D/
    cmake ../../ -DTORQUE_APP_NAME=Torque3D $ADDITIONAL_CMAKE_ARGS
    echo "FOR TESTING:: CFLAGS: $CFLAGS LDFLAGS: $LDFLAGS"
    cmake --build .
    pushd game
    ./Torque3D runTests.cs || true
    popd
    popd
fi

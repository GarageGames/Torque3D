#!/bin/bash
set -e

if [[ $TRAVIS_OS_NAME = osx ]]; then
    BUNDLE_OS_LABEL=OSX
elif [[ $TRAVIS_OS_NAME = linux ]]; then
    BUNDLE_OS_LABEL=Linux
fi
export BUNDLE_NAME=Torque3D-$TRAVIS_TAG-$BUNDLE_OS_LABEL.tar.gz

if [[ $TRAVIS_OS_NAME = linux ]]; then
    sudo chown -R --reference $TRAVIS_BUILD_DIR .
    sudo chmod -R --reference $TRAVIS_BUILD_DIR .
fi

pushd My\ Projects/Torque3D/game
if [[ $TRAVIS_BRANCH = development ]]; then
    echo "$TRAVIS_COMMIT was the last commit when compiling this binary." > VERSION 
fi
tar -zcvf $BUNDLE_NAME *
popd

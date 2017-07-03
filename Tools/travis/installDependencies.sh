#!/bin/bash
set -e

if [[ ${OS_ID:0:6} = ubuntu ]]; then
    apt-get update
    apt-get install -y libgtk-3-dev
    apt-get install -y git build-essential nasm xorg-dev ninja-build gcc-multilib g++-multilib cmake cmake-qt-gui
    apt-get install -y libogg-dev libxft-dev libx11-dev libxxf86vm-dev libopenal-dev libfreetype6-dev libxcursor-dev libxinerama-dev libxi-dev libxrandr-dev libxss-dev libglu1-mesa-dev

elif [[ $TRAVIS_OS_NAME = osx ]]; then
    # brew update
    # brew install sdl2
    echo "No dependencies to install."
fi

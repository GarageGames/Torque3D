cmake_minimum_required (VERSION 2.8)

set(TORQUE_APP_NAME "My Project" CACHE STRING "the app name")
set(TORQUE_APP_VERSION "1000" CACHE STRING "the app version: major * 1000 + minor * 100 + revision * 10.")
set(TORQUE_APP_VERSION_STRING "1.0" CACHE STRING "the app version string")

project(${TORQUE_APP_NAME})

add_subdirectory(Tools/CMake)
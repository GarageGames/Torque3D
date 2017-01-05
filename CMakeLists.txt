cmake_minimum_required (VERSION 2.8.12)

set(TORQUE_APP_NAME "" CACHE STRING "the app name")

set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/temp" CACHE PATH "default install path" FORCE )

if("${TORQUE_APP_NAME}" STREQUAL "")
	message(FATAL_ERROR "Please set TORQUE_APP_NAME first")
endif()

project(${TORQUE_APP_NAME})

add_subdirectory(Tools/CMake)

cmake_minimum_required (VERSION 2.6)

project(theora)

add_definitions(-DTORQUE_OGGTHEORA -DTORQUE_OGGVORIBS)

include_directories(include)
include_directories(${Torque3DEngine_SOURCE_DIR}/lib/libogg/include)

addLibrary(/ theora GLOB_RECURSE)

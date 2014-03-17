cmake_minimum_required (VERSION 2.6)

project(mng)

add_definitions(-DMNG_OPTIMIZE_OBJCLEANUP)

include_directories(.)
include_directories(${Torque3DEngine_SOURCE_DIR}/lib/lpng)
include_directories(${Torque3DEngine_SOURCE_DIR}/lib/zlib)
include_directories(${Torque3DEngine_SOURCE_DIR}/lib/ljpeg)

addLibrary(/ mng GLOB)

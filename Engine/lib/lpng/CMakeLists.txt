cmake_minimum_required (VERSION 2.6)

project(png)

#add_definitions(-DPNG_NO_ASSEMBLER_CODE)

include_directories(.)
include_directories(${Torque3DEngine_SOURCE_DIR}/lib/zlib)

addLibrary(/ png GLOB)

project(vorbis)

add_definitions(-DTORQUE_OGGVORBIS)

include_directories(include)
include_directories(${Torque3DEngine_SOURCE_DIR}/lib/libogg/include)

addLibrary(/ vorbis GLOB_RECURSE)

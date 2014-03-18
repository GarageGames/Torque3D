project(libvorbis)

addLibrary("${libDir}/libvorbis" libvorbis GLOB_RECURSE)

set_property(TARGET libvorbis PROPERTY COMPILE_DEFINITIONS TORQUE_OGGVORBIS)
set_property(TARGET libvorbis PROPERTY INCLUDE_DIRECTORIES ${libDir}/libvorbis/include ${libDir}/libogg/include )

project(libtheora)

addLibrary("${libDir}/libtheora" libtheora GLOB_RECURSE)

set_property(TARGET libtheora PROPERTY COMPILE_DEFINITIONS TORQUE_OGGTHEORA TORQUE_OGGVORIBS)
set_property(TARGET libtheora PROPERTY INCLUDE_DIRECTORIES ${libDir}/libtheora/include ${libDir}/libogg/include)

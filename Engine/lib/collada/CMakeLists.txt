project(collada)

include_directories(include)
include_directories(include/1.4)
include_directories(${Torque3DEngine_SOURCE_DIR}/lib/pcre)
include_directories(${Torque3DEngine_SOURCE_DIR}/lib/tinyxml)

add_definitions(-DDOM_INCLUDE_TINYXML -DPCRE_STATIC)
addLibrary("src/1.4/dom;src/dae;src/modules/LIBXMLPlugin;src/modules/stdErrPlugin;src/modules/STLDatabase" collada GLOB)

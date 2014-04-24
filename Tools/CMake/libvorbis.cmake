project(libvorbis)

addPathRec("${libDir}/libvorbis")

addStaticLib()

addDef(TORQUE_OGGVORBIS)
addInclude(${libDir}/libvorbis/include)
addInclude(${libDir}/libogg/include)

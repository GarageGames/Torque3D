project(libtheora)

addPathRec("${libDir}/libtheora")

addStaticLib()

addDef(TORQUE_OGGTHEORA)
addDef(TORQUE_OGGVORIBS)
addInclude(${libDir}/libogg/include)
addInclude(${libDir}/libtheora/include)

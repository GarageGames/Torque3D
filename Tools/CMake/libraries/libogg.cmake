project(libogg)

addPathRec("${libDir}/libogg")

addStaticLib()

addInclude(${libDir}/libogg/include)

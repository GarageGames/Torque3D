project(collada)

addPath("${libDir}/collada/src/1.4/dom")
addPath("${libDir}/collada/src/dae")
addPath("${libDir}/collada/src/modules/LIBXMLPlugin")
addPath("${libDir}/collada/src/modules/stdErrPlugin")
addPath("${libDir}/collada/src/modules/STLDatabase")

addDef(DOM_INCLUDE_TINYXML)
addDef(PCRE_STATIC)

addInclude(${libDir}/collada/include)
addInclude(${libDir}/collada/include/1.4)
addInclude(${libDir}/pcre)
addInclude(${libDir}/tinyxml)

finishLibrary()

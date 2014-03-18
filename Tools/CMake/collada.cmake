project(collada)

addLibrary("${libDir}/collada/src/1.4/dom;${libDir}/collada/src/dae;${libDir}/collada/src/modules/LIBXMLPlugin;${libDir}/collada/src/modules/stdErrPlugin;${libDir}/collada/src/modules/STLDatabase" collada GLOB)

set_property(TARGET collada PROPERTY COMPILE_DEFINITIONS DOM_INCLUDE_TINYXML PCRE_STATIC)
set_property(TARGET collada PROPERTY INCLUDE_DIRECTORIES ${libDir}/collada/include ${libDir}/collada/include/1.4 ${libDir}/pcre ${libDir}/tinyxml)

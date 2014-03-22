project(pcre)

addStaticLib("${libDir}/pcre")

addDef(PCRE_STATIC)
addDef(HAVE_CONFIG_H)

set_property(TARGET pcre PROPERTY COMPILE_FLAGS       /TP) #/TP = compile as C++

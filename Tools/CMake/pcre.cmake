project(pcre)

addLibrary("${libDir}/pcre" pcre GLOB)

set_property(TARGET pcre PROPERTY COMPILE_DEFINITIONS PCRE_STATIC HAVE_CONFIG_H)
set_property(TARGET pcre PROPERTY COMPILE_FLAGS       /TP) #/TP = compile as C++
set_property(TARGET pcre PROPERTY INCLUDE_DIRECTORIES ${libDir}/pcre)
